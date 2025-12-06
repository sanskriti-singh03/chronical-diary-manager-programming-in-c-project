// CHRONICAL - A Personal Diary Manager
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_TITLE_LENGTH 100
#define MAX_CONTENT_LENGTH 1000
#define DATE_LENGTH 11 
#define BUFFER_SIZE 1024

// Structure to store each diary entry
typedef struct {
    char date[DATE_LENGTH];          
    char *content;                   
    char title[MAX_TITLE_LENGTH];    
} Entry;

// Function declarations
void addEntry(const char *filename);
void viewEntries(const char *filename);
void searchEntries(const char *filename);
void displayQuote();
int getChoice();
void clearInputBuffer();
char* getEntryContent();

int main() {
    const char *filename = "diary_entries.dat";
    int choice;

    printf("\nWelcome to your Personal Diary Manager!\n");
    displayQuote();  

    // Main menu loop
    do {
        printf("\n<-- Main Menu -->\n");
        printf("1. Add a new entry\n");
        printf("2. View all entries\n");
        printf("3. Search entries\n");
        printf("4. Exit\n");
        printf("<--------------->\n");

        choice = getChoice();  

        switch (choice) {
            case 1:
                addEntry(filename);  
                break;
            case 2:
                viewEntries(filename);  
                break;
            case 3:
                searchEntries(filename); 
                break;
            case 4:
                printf("Thank you for using the Personal Diary Manager. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 4);  

    return 0;
}

void addEntry(const char *filename) {
    FILE *file = fopen(filename, "ab"); 
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    Entry newEntry;

    // Get today's date automatically
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(newEntry.date, DATE_LENGTH, "%04d-%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    // Get entry title
    printf("Enter the title (max %d characters): ", MAX_TITLE_LENGTH - 1);
    fgets(newEntry.title, MAX_TITLE_LENGTH, stdin);
    newEntry.title[strcspn(newEntry.title, "\n")] = 0;

    // Get full diary content
    printf("Enter the content (max %d characters).\n", MAX_CONTENT_LENGTH - 1);
    printf("End with a single '.' on a new line.\n");
    newEntry.content = getEntryContent();  
    if (newEntry.content == NULL) {
        fclose(file);
        return;
    }

    // Write entry to file in binary format
    size_t content_len = strlen(newEntry.content) + 1; 
    fwrite(&content_len, sizeof(size_t), 1, file);
    fwrite(newEntry.date, sizeof(newEntry.date), 1, file);
    fwrite(newEntry.title, sizeof(newEntry.title), 1, file);
    fwrite(newEntry.content, sizeof(char), content_len, file);

    free(newEntry.content);
    fclose(file);

    printf("Entry saved successfully!\n");
}

void viewEntries(const char *filename) {
    FILE *file = fopen(filename, "rb"); 
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    Entry entry;
    size_t content_len;
    int count = 0;

    printf("\n--- All Diary Entries ---\n");

    // Read entries one by one
    while (fread(&content_len, sizeof(size_t), 1, file) == 1) {
        fread(entry.date, sizeof(entry.date), 1, file);
        fread(entry.title, sizeof(entry.title), 1, file);

        entry.content = (char *)malloc(content_len);  
        if (entry.content == NULL) {
            perror("Memory allocation failed");
            break;
        }

        fread(entry.content, sizeof(char), content_len, file);

        // Display each entry neatly
        printf("<-------------------------------->\n");
        printf("Date   : %s\n", entry.date);
        printf("Title  : %s\n", entry.title);
        printf("Content:\n%s", entry.content);
        printf("<-------------------------------->\n");

        free(entry.content);
        count++;
    }

    if (count == 0) {
        printf("No entries found.\n");
    }

    fclose(file);
}

void displayQuote() {
    // Fixed list of motivational quotes
    const char *quotes[] = {
        "The best way to get started is to quit talking and begin doing. - Walt Disney",
        "Don't let yesterday take up too much of today. - Will Rogers",
        "It's not whether you get knocked down, it's whether you get up. - Vince Lombardi",
        "If you are working on something exciting, it will keep you motivated. - Unknown",
        "Success is not in what you have, but who you are. - Bo Bennett"
    };

    // Choose a random quote
    size_t num_quotes = sizeof(quotes) / sizeof(quotes[0]);
    srand((unsigned int)time(NULL));
    int index = rand() % num_quotes;

    printf("\nMotivational Quote:\n%s\n", quotes[index]);
}

char* getEntryContent() {
    
    char *buffer = NULL;
    size_t size = 0;   
    size_t len  = 0;   
    char line[BUFFER_SIZE];

    printf("Enter your content (end with a single '.' on its own line):\n");

    while (1) {
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        // Stop when user enters only "."
        if (strcmp(line, ".") == 0) {
            break;
        }

        size_t line_len = strlen(line);

        
        if (len + line_len + 2 > size) {
            size_t new_size = (len + line_len + 2) * 2;
            char *new_buffer = realloc(buffer, new_size);
            if (new_buffer == NULL) {
                free(buffer);
                perror("Memory allocation failed");
                return NULL;
            }
            buffer = new_buffer;
            size = new_size;
        }

    
        memcpy(buffer + len, line, line_len);
        len += line_len;

        buffer[len++] = '\n'; 
    }

    // Finalize string
    if (buffer == NULL) {
        buffer = malloc(1);
        buffer[0] = '\0';
    } else {
        buffer[len] = '\0';
    }

    return buffer;
}

void searchEntries(const char *filename) {
    FILE *file = fopen(filename, "rb"); 
    if (file == NULL) {
        printf("No entries found.\n");
        return;
    }

    char search_term[BUFFER_SIZE];
    printf("Enter a keyword or date (YYYY-MM-DD) to search: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = 0;

    if (strlen(search_term) == 0) {
        printf("Search term cannot be empty.\n");
        fclose(file);
        return;
    }

    Entry entry;
    size_t content_len;
    int found = 0;

    printf("\n--- Search Results ---\n");

    // Check each entry for matches
    while (fread(&content_len, sizeof(size_t), 1, file) == 1) {
        fread(entry.date, sizeof(entry.date), 1, file);
        fread(entry.title, sizeof(entry.title), 1, file);

        entry.content = (char *)malloc(content_len);

        fread(entry.content, sizeof(char), content_len, file);

        // Search in date, title, and content
        if (strstr(entry.date,  search_term) ||
            strstr(entry.title, search_term) ||
            strstr(entry.content, search_term)) {

            printf("----------------------------------\n");
            printf("Date   : %s\n", entry.date);
            printf("Title  : %s\n", entry.title);
            printf("Content:\n%s", entry.content);
            printf("----------------------------------\n");
            found++;
        }

        free(entry.content);
    }

    if (found == 0)
        printf("No matching entries found.\n");

    fclose(file);
}

int getChoice() {
    // Ensures only a valid number is accepted
    int choice;
    printf("Enter your choice: ");
    while (scanf("%d", &choice) != 1) {
        printf("Invalid input. Please enter a number: ");
        clearInputBuffer();
    }
    clearInputBuffer(); 
    return choice;
}

// Clears leftover characters from input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
