
#include <stm32l432xx.h>
#include "ff.h"
#include "commands.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

#include "../include/ui.h"


// Data structure for the mounted file system.
FATFS fs_storage;

typedef union {
    struct {
        unsigned int bisecond:5; // seconds divided by 2
        unsigned int minute:6;
        unsigned int hour:5;
        unsigned int day:5;
        unsigned int month:4;
        unsigned int year:7;
    };
} fattime_t;

// Current time in the FAT file system format.
static fattime_t fattime;

void set_fattime(int year, int month, int day, int hour, int minute, int second)
{
    fattime_t newtime;
    newtime.year = year - 1980;
    newtime.month = month;
    newtime.day = day;
    newtime.hour = hour;
    newtime.minute = minute;
    newtime.bisecond = second/2;
    int len = sizeof newtime;
    memcpy(&fattime, &newtime, len);
}

void advance_fattime(void)
{
    fattime_t newtime = fattime;
    newtime.bisecond += 1;
    if (newtime.bisecond == 30) {
        newtime.bisecond = 0;
        newtime.minute += 1;
    }
    if (newtime.minute == 60) {
        newtime.minute = 0;
        newtime.hour += 1;
    }
    if (newtime.hour == 24) {
        newtime.hour = 0;
        newtime.day += 1;
    }
    if (newtime.month == 2) {
        if (newtime.day >= 29) {
            int year = newtime.year + 1980;
            if ((year % 1000) == 0) { // we have a leap day in 2000
                if (newtime.day > 29) {
                    newtime.day -= 28;
                    newtime.month = 3;
                }
            } else if ((year % 100) == 0) { // no leap day in 2100
                if (newtime.day > 28)
                newtime.day -= 27;
                newtime.month = 3;
            } else if ((year % 4) == 0) { // leap day for other mod 4 years
                if (newtime.day > 29) {
                    newtime.day -= 28;
                    newtime.month = 3;
                }
            }
        }
    } else if (newtime.month == 9 || newtime.month == 4 || newtime.month == 6 || newtime.month == 10) {
        if (newtime.day == 31) {
            newtime.day -= 30;
            newtime.month += 1;
        }
    } else {
        if (newtime.day == 0) { // cannot advance to 32
            newtime.day = 1;
            newtime.month += 1;
        }
    }
    if (newtime.month == 13) {
        newtime.month = 1;
        newtime.year += 1;
    }

    fattime = newtime;
}

uint32_t get_fattime(void)
{
    union FattimeUnion {
        fattime_t time;
        uint32_t value;
    };

    union FattimeUnion u;
    u.time = fattime;
    return u.value;
}



void print_error(FRESULT fr, const char *msg)
{
    const char *errs[] = {
            [FR_OK] = "Success",
            [FR_DISK_ERR] = "Hard error in low-level disk I/O layer",
            [FR_INT_ERR] = "Assertion failed",
            [FR_NOT_READY] = "Physical drive cannot work",
            [FR_NO_FILE] = "File not found",
            [FR_NO_PATH] = "Path not found",
            [FR_INVALID_NAME] = "Path name format invalid",
            [FR_DENIED] = "Permision denied",
            [FR_EXIST] = "Prohibited access",
            [FR_INVALID_OBJECT] = "File or directory object invalid",
            [FR_WRITE_PROTECTED] = "Physical drive is write-protected",
            [FR_INVALID_DRIVE] = "Logical drive number is invalid",
            [FR_NOT_ENABLED] = "Volume has no work area",
            [FR_NO_FILESYSTEM] = "Not a valid FAT volume",
            [FR_MKFS_ABORTED] = "f_mkfs aborted",
            [FR_TIMEOUT] = "Unable to obtain grant for object",
            [FR_LOCKED] = "File locked",
            [FR_NOT_ENOUGH_CORE] = "File name is too large",
            [FR_TOO_MANY_OPEN_FILES] = "Too many open files",
            [FR_INVALID_PARAMETER] = "Invalid parameter",
    };
    if (fr < 0 || fr >= sizeof errs / sizeof errs[0])
        printf("%s: Invalid error\n", msg);
    else
        printf("%s: %s\n", msg, errs[fr]);
}


int to_int(char *start, char *end, int base)
{
    int n = 0;
    for( ; start != end; start++)
        n = n * base + (*start - '0');
    return n;
}

static const char *month_name[] = {
        [1] = "Jan",
        [2] = "Feb",
        [3] = "Mar",
        [4] = "Apr",
        [5] = "May",
        [6] = "Jun",
        [7] = "Jul",
        [8] = "Aug",
        [9] = "Sep",
        [10] = "Oct",
        [11] = "Nov",
        [12] = "Dec",
};

void set_fattime(int,int,int,int,int,int);


void date(int argc, char *argv[])
{
    if (argc == 2) {
        char *d = argv[1];
        if (strlen(d) != 14) {
            printf("Date format: YYYYMMDDHHMMSS\n");
            return;
        }
        for(int i=0; i<14; i++)
            if (d[i] < '0' || d[i] > '9') {
                printf("Date format: YYYMMDDHHMMSS\n");
                return;
            }
        int year = to_int(d, &d[4], 10);
        int month = to_int(&d[4], &d[6], 10);
        int day   = to_int(&d[6], &d[8], 10);
        int hour  = to_int(&d[8], &d[10], 10);
        int minute = to_int(&d[10], &d[12], 10);
        int second = to_int(&d[12], &d[14], 10);
        set_fattime(year, month, day, hour, minute, second);
        return;
    }
    int integer = get_fattime();
    union {
        int integer;
        fattime_t ft;
    } u;
    u.integer = integer;
    fattime_t ft = u.ft;
    int year = ft.year + 1980;
    int month = ft.month;
    printf("%d-%s-%02d %02d:%02d:%02d\n",
            year, month_name[month], ft.day, ft.hour, ft.minute, ft.bisecond*2);
}


//DEBUGGING FUNCTION
void log_to_sd(const char *message)
{
    FIL fil;        /* File object */
    FRESULT fr;     /* FatFs return code */
    const char *filename = "log";
    // const char *message = "BANG DELETED!!.\n";
    
    // Open the file in append mode (or create if it doesn't exist)
    fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND);
    if (fr) {
        printf("Error opening %s: %d\n", filename, fr);
        return;
    }
    
    UINT wlen;
    fr = f_write(&fil, message, strlen(message), &wlen);
    if (fr) {
        printf("Error writing to %s: %d\n", filename, fr);
    }
    
    // Flush the file to ensure data is written
    f_sync(&fil);
    
    // Close the file
    f_close(&fil);
}


//DELETE A SET
void delete_file(const char *filename){
    FRESULT fr;
    //try to delete
    fr = f_unlink(filename);

    if(fr == FR_OK){
        // log_to_sd("working!!!");
    }
    
}

//listing all the decks in the directory
int get_decks(char decks[MAX_DECKS][MAX_NAME_SIZE])
{
    FRESULT res;
    DIR dir;
    static FILINFO fno;
    const char *path = "";
    int i = 0;
        res = f_opendir(&dir, path);                       /* Open the directory */
        if (res != FR_OK) {
            return 0;
        }
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */

            // check if the file has a .txt extension
            char *ext = strrchr(fno.fname, '.'); //last .
            if (ext == NULL || ext == fno.fname) { //no extension or starts with .
                if (i > MAX_DECKS) break; //not exceeding array
                strncpy(decks[i], fno.fname, strlen(fno.fname));
                i++;
            }
        }
        f_closedir(&dir);
    return i;
}

//NECESSARY FOR USING SD, WE NEED THIS!!!
void mount()
{
    FATFS *fs = &fs_storage;
    if (fs->id != 0) {
        return;
    }
    int res = f_mount(fs, "", 1);
    if (res != FR_OK){
    }
}

//parse Json to structures NEED THIS!!!
int parseJSON_file(const char* filename, struct deck* deck){
    FIL fil;
    FRESULT fr;
    char jsonBuffer[4096]; //can change, buffer size
    UINT bytesRead;

    //open file for reading
    fr = f_open(&fil, filename, FA_READ);
    if (fr){
        // log_to_sd("Error opening file!");
        return -1;
    }

    //read content into a buffer
    fr = f_read(&fil, jsonBuffer, sizeof(jsonBuffer)-1, &bytesRead);
    f_close(&fil);

    //ensure data actually read
    if(fr || bytesRead == 0){
        // log_to_sd("Error reading file!");
        return -1;
    }

    jsonBuffer[bytesRead] = '\0'; //null terminate

    //start JSON parsing
    cJSON * json = cJSON_Parse(jsonBuffer);
    if (!json){
        // log_to_sd("Error parsing JSON!");
        return -1;
    }

    //GETTING AND SAVING NAME IN DECK
    cJSON* name = cJSON_GetObjectItem(json, "flashcardSetName");

    if(cJSON_IsString(name)){
        strncpy(deck->name, name->valuestring, sizeof(deck->name)-1);
        deck->name[sizeof(deck->name) - 1] = '\0'; //null terminate
    }

    //GET FLASHCARD ARRAY AND SAVE
    cJSON* flashcards = cJSON_GetObjectItem(json, "flashcards");
    if(cJSON_IsArray(flashcards)){
        cJSON * flashcard;
        int index = 0;
        cJSON_ArrayForEach(flashcard, flashcards){
            if (index >= MAX_CARDS_PER_DECK){
                // log_to_sd("warning mx flashcards reached!");
                break;
            }

            cJSON* term = cJSON_GetObjectItem(flashcard, "term");
            cJSON* definition = cJSON_GetObjectItem(flashcard, "definition");

            if(cJSON_IsString(term) && cJSON_IsString(definition)) {
                strncpy(deck->cards[index].front, term->valuestring, MAX_FRONT_SIZE -1);
                strncpy(deck->cards[index].back, definition->valuestring, MAX_FRONT_SIZE -1);

                deck->cards[index].front[MAX_FRONT_SIZE - 1] = '\0';
                deck->cards[index].back[MAX_BACK_SIZE - 1] = '\0';

                index++;

            }

        }
        deck->num_cards = index;
        
    }
    else{
        deck->num_cards = 0;
    }

    //cleanup
    cJSON_Delete(json);
    return 0;

    
}

struct commands_t cmds[] = {
        { "date", date },
        // { "ls", ls },
        { "mount", mount },

};

