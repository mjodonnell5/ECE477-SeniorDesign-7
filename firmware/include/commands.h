#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "../include/ui.h"

struct commands_t {
    const char *cmd;
    void      (*fn)(int argc, char *argv[]);
};

void command_shell(void);
void mount();
// void log_to_sd();
void log_to_sd(const char *message); //debugging purposes
void delete_file(const char *filename); // deleting a file off the sd card
void cat(const char *filename); // displaying a file contents
void parseJson(const char *filename); //parsing a json file
void ls(int argc, char *argv[]);
int parseJSON_file(const char* filename, struct deck* deck);
int get_decks(char decks[MAX_DECKS][MAX_NAME_SIZE]);
void updateMetadata_newSet(const char* filename);
void updateMetadata_date(const char* filename);
void updateMetadata_delete(const char* filename);


#endif /* __COMMANDS_H_ */
