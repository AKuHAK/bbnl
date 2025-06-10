#include "common.h"
#include "config.h"
#include "loader.h"
#include <ctype.h>
#include <fcntl.h>
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Arguments
static char neutrinoPath[] = PFS_MOUNTPOINT "/neutrino/neutrino.elf";
static char isoArgument[] = "dvd";
static char bsdArgument[] = "bsd";
// Neutrino bsd values
#define BSD_ATA "ata"

// An entry in ArgumentList
typedef struct Argument {
  char *arg;   // Argument
  char *value; // Argument value
  int isDisabled;

  struct Argument *prev; // Previous target in the list
  struct Argument *next; // Next target in the list
} Argument;

// A linked list of options from config file
typedef struct {
  int total;       // Total number of arguments
  Argument *first; // First target
  Argument *last;  // Last target
} ArgumentList;

ArgumentList *loadLaunchArgumentLists(char *titlePath);
void appendArgument(ArgumentList *target, Argument *arg);
Argument *newArgument(const char *argName, char *value);

// Assembles argument lists into argv for loader.elf.
// Expects argv to be initialized with at least (arguments->total+1) elements.
int assembleArgv(ArgumentList *arguments, char *argv[]) {
  Argument *curArg = arguments->first;
  int argCount = 1; // argv[0] is always neutrino.elf
  int argSize = 0;

  argv[0] = neutrinoPath;
  while (curArg != NULL) {
    if (!curArg->isDisabled) {
      argSize = strlen(curArg->arg) + strlen(curArg->value) + 3; // + \0, = and -
      char *value = calloc(sizeof(char), argSize);

      if (!strlen(curArg->value))
        snprintf(value, argSize, "-%s", curArg->arg);
      else
        snprintf(value, argSize, "-%s=%s", curArg->arg, curArg->value);

      argv[argCount] = value;
      argCount++;
    }
    curArg = curArg->next;
  }

  // Free unused memory
  if (argCount != arguments->total)
    argv = realloc(argv, argCount * sizeof(char *));

  return argCount;
}

// Launches the ISO via Neutrino.
// Assumes parititon containing Neutrino as mounted as pfs0.
int launchNeutrino(char *fileName, DiscType type) {
  if (tryFile(neutrinoPath)) {
    printf("ERROR: neutrino.elf is inaccessible\n");
    return -ENOENT;
  }

  // Build full ISO path
  int pathLength = sizeof(PFS_MOUNTPOINT) + 5 + strlen(fileName);
  char *filePath = calloc(sizeof(char), pathLength);
  switch (type) {
  case DISC_TYPE_CD:
    snprintf(filePath, pathLength, "%s/CD/%s", PFS_MOUNTPOINT, fileName);
    break;
  case DISC_TYPE_DVD:
    snprintf(filePath, pathLength, "%s/DVD/%s", PFS_MOUNTPOINT, fileName);
    break;
  default:
    printf("ERROR: Unknown disc type\n");
    return -1;
  }

  // Initialize arugment list from config files on exFAT partition
  ArgumentList *arguments = loadLaunchArgumentLists(fileName);

  // Append bsd and ISO path
  appendArgument(arguments, newArgument(bsdArgument, BSD_ATA));
  appendArgument(arguments, newArgument(isoArgument, filePath));
  // Use quickboot to reduce load times
  appendArgument(arguments, newArgument("qb", ""));

  // Assemble argv
  char **argv = malloc(((arguments->total) + 1) * sizeof(char *));
  int argCount = assembleArgv(arguments, argv);

  printf("Launching %s with arguments:\n", filePath);
  for (int i = 0; i < argCount; i++) {
    printf("%d: %s\n", i + 1, argv[i]);
  }
  printf("ERROR: Failed to load %s: %d\n", neutrinoPath, LoadELFFromFile(argCount, argv));
  return -ENOENT;
}

//
// NHDDL options file handling
//

#define BASE_CONFIG_DIR "nhddl"
#define GLOBAL_OPTIONS_PATH "global.yaml"

int loadArgumentList(ArgumentList *options, char *filePath);
int parseOptionsFile(ArgumentList *result, FILE *file);
void freeArgumentList(ArgumentList *result);

// Generates ArgumentList from global config file located at targetMounpoint (usually ISO full path)
int getGlobalLaunchArguments(ArgumentList *result) {
  char targetPath[PATH_MAX];
  snprintf(targetPath, PATH_MAX, "%s/%s/%s", PFS_MOUNTPOINT, BASE_CONFIG_DIR, GLOBAL_OPTIONS_PATH);
  int ret = loadArgumentList(result, targetPath);
  Argument *curArg = result->first;
  while (curArg != NULL) {
    curArg = curArg->next;
  }
  return ret;
}

// Generates ArgumentList from global and title-specific config file
int getTitleLaunchArguments(ArgumentList *result, char *titleName) {
  // Extract ISO name from the title name
  char *fileext = strrchr(titleName, '.');
  if (fileext != NULL) {
    *fileext = '\0';
  }

  printf("Looking for title-specific config for %s\n", titleName);
  char targetPath[PATH_MAX];
  snprintf(targetPath, PATH_MAX, "%s/%s", PFS_MOUNTPOINT, BASE_CONFIG_DIR);
  // Determine actual title options file from config directory contents
  DIR *directory = opendir(targetPath);
  if (directory == NULL) {
    printf("ERROR: Can't open %s\n", targetPath);
    return -ENOENT;
  }
  targetPath[0] = '\0';

  // Find title config in config directory
  struct dirent *entry;
  while ((entry = readdir(directory)) != NULL) {
    if (entry->d_type != DT_DIR) {
      // Find file that starts with ISO name (without the extension)
      if (!strncmp(entry->d_name, titleName, strlen(titleName))) {
        snprintf(targetPath, PATH_MAX, "%s/%s/%s", PFS_MOUNTPOINT, BASE_CONFIG_DIR, entry->d_name);
        break;
      }
    }
  }
  closedir(directory);

  if (targetPath[0] == '\0') {
    printf("Title-specific config not found\n");
    return 0;
  }

  // Load arguments
  printf("Loading title-specific config from %s\n", targetPath);
  int ret = loadArgumentList(result, targetPath);
  if (ret) {
    printf("ERROR: Failed to load argument list: %d\n", ret);
  }

  return 0;
}

// Parses options file into ArgumentList
int loadArgumentList(ArgumentList *options, char *filePath) {
  // Open options file
  FILE *file = fopen(filePath, "r");
  if (file == NULL) {
    printf("ERROR: Failed to open %s\n", filePath);
    return -ENOENT;
  }

  // Initialize ArgumentList
  options->total = 0;
  options->first = NULL;
  options->last = NULL;

  // Parse options file
  if (parseOptionsFile(options, file)) {
    fclose(file);
    freeArgumentList(options);
    return -EIO;
  }

  fclose(file);
  return 0;
}

// Parses file into ArgumentList. Result may contain parsed arguments even if an error is returned.
// Adds mountpoint with to arguments values that start with \ or /
int parseOptionsFile(ArgumentList *result, FILE *file) {
  // Our lines will mostly consist of file paths, which aren't likely to exceed 300 characters due to 255 character limit in exFAT path component
  char lineBuffer[PATH_MAX + 1];
  lineBuffer[0] = '\0';
  int startIdx;
  int substrIdx;
  int argEndIdx;
  int isDisabled = 0;

  while (fgets(lineBuffer, PATH_MAX, file)) { // fgets reutrns NULL if EOF or an error occurs
    startIdx = 0;
    isDisabled = 0;
    argEndIdx = 0;

    //
    // Parse argument
    //
    while (isspace((unsigned char)lineBuffer[startIdx])) {
      startIdx++; // Advance line index until we read a non-whitespace character
    }
    // Ignore comment lines
    if (lineBuffer[startIdx] == '#')
      continue;

    // Try to find ':' until line ends
    substrIdx = startIdx;
    while (lineBuffer[substrIdx] != ':' && lineBuffer[substrIdx] != '\0') {
      if (lineBuffer[substrIdx] == '$') {
        // Handle disabled argument
        isDisabled = 1;
        startIdx = substrIdx + 1;
      } else if (isspace((unsigned char)lineBuffer[startIdx])) {
        // Ignore whitespace by advancing start index to ignore this character
        startIdx = substrIdx + 1;
      }
      substrIdx++;
    }

    // If EOL is reached without finding ':', skip to the next line
    if (lineBuffer[substrIdx] == '\0') {
      goto next;
    }

    // Mark the end of argument name before removing trailing whitespace
    argEndIdx = substrIdx;

    // Remove trailing whitespace
    while (isspace((unsigned char)lineBuffer[substrIdx - 1])) {
      substrIdx--;
    }

    // Copy argument to argName
    char *argName = calloc(sizeof(char), substrIdx - startIdx + 1);
    strncpy(argName, &lineBuffer[startIdx], substrIdx - startIdx);
    substrIdx = argEndIdx;

    //
    // Parse value
    //
    startIdx = substrIdx + 1;
    // Advance line index until we read a non-whitespace character or return at EOL
    while (isspace((unsigned char)lineBuffer[startIdx])) {
      if (lineBuffer[startIdx] == '\0') {
        free(argName);
        goto next;
      }
      startIdx++;
    }

    // Try to read value until we reach a comment, a new line, or the end of string
    substrIdx = startIdx;
    while (lineBuffer[substrIdx] != '#' && lineBuffer[substrIdx] != '\r' && lineBuffer[substrIdx] != '\n' && lineBuffer[substrIdx] != '\0') {
      substrIdx++;
    }

    // Remove trailing whitespace
    while ((substrIdx > startIdx) && isspace((unsigned char)lineBuffer[substrIdx - 1])) {
      substrIdx--;
    }

    Argument *arg = newArgument(argName, "");
    free(argName);
    arg->isDisabled = isDisabled;

    // Allocate memory for the argument value
    size_t valueLength = substrIdx - startIdx;
    if ((lineBuffer[startIdx] == '/') || (lineBuffer[startIdx] == '\\')) {
      // Add device mountpoint to argument value if path starts with \ or /
      arg->value = calloc(sizeof(char), valueLength + 1 + strlen(PFS_MOUNTPOINT));
      strcpy(arg->value, PFS_MOUNTPOINT);
    } else {
      arg->value = calloc(sizeof(char), valueLength + 1);
    }

    // Copy the value and add argument to the list
    strncat(arg->value, &lineBuffer[startIdx], valueLength);
    appendArgument(result, arg);

  next:
  }
  if (ferror(file) || !feof(file)) {
    printf("ERROR: Failed to read config file\n");
    return -EIO;
  }

  return 0;
}

// Completely frees Argument and returns pointer to a previous argument in the list
Argument *freeArgument(Argument *arg) {
  Argument *prev = NULL;
  if (arg->arg)
    free(arg->arg);
  if (arg->value)
    free(arg->value);
  if (arg->prev)
    prev = arg->prev;

  free(arg);
  return prev;
}

// Completely frees ArgumentList. Passed pointer will not be valid after this function executes
void freeArgumentList(ArgumentList *result) {
  Argument *tArg = result->last;
  while (tArg != NULL) {
    tArg = freeArgument(tArg);
  }
  result->first = NULL;
  result->last = NULL;
  result->total = 0;
  free(result);
}

// Makes and returns a deep copy of src without prev/next pointers.
Argument *copyArgument(Argument *src) {
  // Do a deep copy for argument and value
  Argument *copy = calloc(sizeof(Argument), 1);
  copy->isDisabled = src->isDisabled;
  if (src->arg)
    copy->arg = strdup(src->arg);
  if (src->value)
    copy->value = strdup(src->value);
  return copy;
}

// Replaces argument and value in dst, freeing arg and value.
// Keeps next and prev pointers.
void replaceArgument(Argument *dst, Argument *src) {
  // Do a deep copy for argument and value
  if (dst->arg)
    free(dst->arg);
  if (dst->value)
    free(dst->value);
  dst->isDisabled = src->isDisabled;
  if (src->arg)
    dst->arg = strdup(src->arg);
  if (src->value)
    dst->value = strdup(src->value);
}

// Creates new Argument with passed argName and value.
// Copies both argName and value
Argument *newArgument(const char *argName, char *value) {
  Argument *arg = malloc(sizeof(Argument));
  arg->isDisabled = 0;
  arg->prev = NULL;
  arg->next = NULL;
  if (argName)
    arg->arg = strdup(argName);
  if (value)
    arg->value = strdup(value);

  return arg;
}

// Appends arg to the end of target
void appendArgument(ArgumentList *target, Argument *arg) {
  target->total++;

  if (!target->first) {
    target->first = arg;
  } else {
    target->last->next = arg;
    arg->prev = target->last;
  }
  target->last = arg;
}

// Merges two lists into one, ignoring arguments in the second list that already exist in the first list.
// All arguments merged from the second list are a deep copy of arguments in source lists.
// Expects both lists to be initialized.
void mergeArgumentLists(ArgumentList *list1, ArgumentList *list2) {
  Argument *curArg1;
  Argument *curArg2 = list2->first;
  int isDuplicate = 0;

  // Copy arguments from the second list into result
  while (curArg2 != NULL) {
    isDuplicate = 0;
    // Look for duplicate arguments in the first list
    curArg1 = list1->first;
    while (curArg1 != NULL) {
      // If result already contains argument with the same name, skip it
      if (!strcmp(curArg2->arg, curArg1->arg)) {
        isDuplicate = 1;
        // If argument is disabled and has no value
        if (curArg1->isDisabled && (curArg1->value[0] == '\0')) {
          // Replace element in list1 with disabled element from list2
          replaceArgument(curArg1, curArg2);
          curArg1->isDisabled = 1;
        }
        break;
      }
      curArg1 = curArg1->next;
    }
    // If no duplicate was found, insert the argument
    if (!isDuplicate) {
      Argument *copy = copyArgument(curArg2);
      appendArgument(list1, copy);
    }
    curArg2 = curArg2->next;
  }
}

// Loads both global and title launch arguments, returning pointer to a merged list
ArgumentList *loadLaunchArgumentLists(char *titleName) {
  int res = 0;
  // Initialize global argument list
  ArgumentList *globalArguments = calloc(sizeof(ArgumentList), 1);
  if ((res = getGlobalLaunchArguments(globalArguments))) {
    printf("WARN: Failed to load global launch arguments: %d\n", res);
  }
  // Initialize title list and merge global into it
  ArgumentList *titleArguments = calloc(sizeof(ArgumentList), 1);
  if ((res = getTitleLaunchArguments(titleArguments, titleName))) {
    printf("WARN: Failed to load title arguments: %d\n", res);
  }

  if (titleArguments->total != 0) {
    // Merge lists
    mergeArgumentLists(titleArguments, globalArguments);
    freeArgumentList(globalArguments);
    return titleArguments;
  }
  // If there are no title arguments, use global arguments directly
  free(titleArguments);
  return globalArguments;
}
