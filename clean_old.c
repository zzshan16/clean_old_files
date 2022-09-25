#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define DURATION 2592000 //minimum duration (in seconds) since last modified date before a file is pruned

/*
 *READ BEFORE COMPILING:
 *
 *The current implementation of this program will create directories
 *(within the directory to be sorted)
 *and move files, whose last modified date is more than DURATION seconds ago,
 *to those newly created (or existing) directories.
 *There will be one directory created per month. 
 *
 *here is a command to find all files excluding directories: "ls -p | grep -v /$"
 *(use it to replace line 78)
 *
 *Possible issues include, but are not limited to,
 *strange file names, long file names, errors running commands
 */

int is_dir(char* dir){
  struct stat buf;
  if (stat(dir, &buf) != 0) return 0;
  return S_ISDIR(buf.st_mode);
}

int main(int argc, char** argv){
  unsigned long long t = time(NULL);
  if (t <= DURATION){
    puts("error: duration must be set to a value lesser than current time");
    return 4;
  }
  t -= DURATION;
  char* dir_to_sort;
  if (argc == 2){
    if (strlen(*(argv+1)) > 200){
      puts("error: path too long");
      return 3;
    }
    dir_to_sort = *(argv+1);
  }
  else {
    dir_to_sort = ".";
  }
  char *line = (char *)malloc(1024*sizeof(char));
  if (!line) {
  malloc_failed:
    puts("error: malloc() failed");
    return 2;
  }

  if (!is_dir(dir_to_sort)){
    sprintf(line, "error: dir \"%s\" not found or is not a directory", dir_to_sort);
    puts(line);
    return 3;
  }
  int user_input;

 input_loop:
  printf("The directory to be sorted is \"%s\"\n", dir_to_sort);
  puts("Enter 1 to continue, 2 to list files in directory, or 3 or CTRL-C to exit.");
  scanf("%1d", &user_input);
  switch(user_input){
  case 1:
    break;
  case 3:
    return 0;
  case 2:
    sprintf(line, "ls %s", dir_to_sort);
    system(line);
  default:
    while ((user_input = getchar()) != '\n');
    goto input_loop;
  }
  sprintf(line, "cd %s && ls ./*.png ./*.jpg ./*.gif ./*.webm ./*.mkv ./*.mp4 ./*.mp3 ./*.pdf ./*.wav ./*.zip ./*.rar ./*.7z ./*.tar.gz 2> /dev/null", dir_to_sort); //add/remove/replace the extensions in this string to control what is being sorted
  puts(line); // the "./" is to prevent bad filenames causing errors
  FILE* names = popen(line, "r");
  if (!names) {
    puts("error: popen() failed");
    return 1;
  }
  char* command_string = malloc(sizeof(char)*strlen(line)+200);
  while(fgets(line, 1024, names)){
    *(line + strlen(line) -1) = '\0'; //replaces newline character with null terminator
    sprintf(command_string, "stat -c %%Y \"%s/%s\"", dir_to_sort, line);
    puts(command_string);
    FILE* temp_age = popen(command_string, "r");
    unsigned long long age;
    if (fscanf(temp_age, "%llu", &age) == EOF){
      puts("error:fscanf() failed");
      pclose(temp_age);
      free(command_string);
      return 3;
      }
    if (age < t){
      *(command_string + 9) = 'y';
      puts(command_string);
      FILE* date = popen(command_string, "r");
      char* date_string = malloc(9*sizeof(char));
      if (!date_string) goto malloc_failed;
      fgets(date_string, 8, date);//it works?
      sprintf(command_string, "mkdir \"%s/%s\" 2>/dev/null", dir_to_sort, date_string);
      puts(command_string);
      system(command_string);
      sprintf(command_string, "mv \"%s/%s\" \"%s/%s\"", dir_to_sort, line, dir_to_sort, date_string);
      puts(command_string);
      system(command_string);
      free(date_string);
      pclose(date);
    }
    pclose(temp_age);
    
  }
  free(command_string);
  free(line);
  pclose(names);
  return 0;
}
