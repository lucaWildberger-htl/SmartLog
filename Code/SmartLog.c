/*---------------------------------------------------
   _____                      __  __
  / ___/____ ___  ____ ______/ /_/ /   ____  ____ _
  \__ \/ __ `__ \/ __ `/ ___/ __/ /   / __ \/ __ `/
 ___/ / / / / / / /_/ / /  / /_/ /___/ /_/ / /_/ /
/____/_/ /_/ /_/\__,_/_/   \__/_____/\____/\__, /
                                          /____/
Zum Entfernen von Botzugriffen aus Webserver-Logs
-----------------------------------------------------
Creator: Luca Wildberger
-----------------------------------------------------
Date: 09.08.2024
-----------------------------------------------------
Version: SmartLog v1.0.1 
-----------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#elif __linux__
#include <unistd.h>
#endif
#include <stdlib.h>


// --- Globale Variablen ---
int isHelpParamReq = 0;
int isFileParamReq = 0;
int countPatternParamReq = 0;
int isDebugParamReq = 0;
int isClearLogReq = 0;
int isNoOutputReq = 0;
char *paramFileName;
char *pattern[5];

// --- Methode zum Überprüfen ob eine Datei richtig geöffnet wurde ---
int checkFile(FILE *file)
{
    if (file == NULL)
    {
        perror("Fehler beim Öffnen der Datei");
        exit(0);
    }
    return 0;
}

// --- Befüllung der Check-Variablen und Nutzvariablen mit den Parametern ---
int getParam(int count, char *params[])
{
    for (int i = 1; i < count; i++)
    {
        if (strcmp(params[i], "-help") == 0 || strcmp(params[i], "-?") == 0 || strcmp(params[i], "/help") == 0 || strcmp(params[i], "/?") == 0)
        {
            isHelpParamReq = 1;
            return 1;
        }
        else if (strcmp(params[i], "-F") == 0 || strcmp(params[i], "-f") == 0 || strcmp(params[i], "/F") == 0 || strcmp(params[i], "/f") == 0)
        {
            isFileParamReq = 1;

            if (i + 1 < count && params[i + 1][0] != '-')
                paramFileName = params[i + 1];
            else
            {
                return 0;
            }
        }
        else if (strcmp(params[i], "-V") == 0 || strcmp(params[i], "-v") == 0 || strcmp(params[i], "/V") == 0 || strcmp(params[i], "/v") == 0)
        {
            isDebugParamReq = 1;
        }
        else if (strcmp(params[i], "-P") == 0 || strcmp(params[i], "-p") == 0 || strcmp(params[i], "/P") == 0 || strcmp(params[i], "/p") == 0)
        {
            countPatternParamReq++;

            if (countPatternParamReq > 5)
            {
                return 0;
            }

            if (i + 1 < count && params[i + 1][0] != '-' && params[i + 1][0] != '/')
            {
                pattern[countPatternParamReq - 1] = params[i + 1];
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(params[i], "-cl") == 0 || strcmp(params[i], "/cl") == 0)
        {
            isClearLogReq = 1;
        }
        else if (strcmp(params[i], "-no") == 0 || strcmp(params[i], "/no") == 0)
        {
            isNoOutputReq = 1;
        }
        else if (params[i][0] == '-')
        {
            return 0;
        }
        else
        {
            if (i > 1 && params[i - 1][0] != '-')
            {
                return 0;
            }
        }
    }
    return 1;
}

// --- Methode zur Überprüfung ob erhaltener Zugriff von einem Bot ausgeführt wurde ---
int contains_pattern(const char *str, int mode)
{
    const char *str2 = str;                   // Nur für Suche nach bot
    const char *pattern_robot = "robots.txt"; // Nur für Suche nach bot
    int reValuePattern = 0;

    if (mode)
    {
        int pattern_bot_WasFound = 0;
        int pattern_robot_WasFound = 0;

        // --- Durchsucht Datei nach "bot" ---
        while (*str)
        {
            if (strncasecmp(str, pattern[0], strlen(pattern[0])) == 0)
            {
                pattern_bot_WasFound = 1;
            }
            str++;
        }

        // --- Durchsucht Datei nach "robots.txt" ---
        while (*str2)
        {
            if (!pattern_robot_WasFound && strncasecmp(str2, pattern_robot, strlen(pattern_robot)) == 0)
            {
                pattern_robot_WasFound = 1;
                str2 = str2 + 10;
            }
            else if (pattern_robot_WasFound)
            {
                if (strncasecmp(str2, pattern[0], strlen(pattern[0])) == 0) // Falls "robots.txt" gefunden, wird nach weiterem "bot" gesucht
                {
                    return 1;
                }
            }
            str2++;
        }

        if (pattern_bot_WasFound && !pattern_robot_WasFound) // Bot-Zugriff, weil "bot" aber kein "robots.txt" in Zeile
        {
            return 1;
        }
    }
    else
    {
        int foundPattern[countPatternParamReq];

        for (int i = 0; i < countPatternParamReq; i++)
        {
            foundPattern[i] = 0;
        }

        while (*str)
        {
            for (int i = 0; i < countPatternParamReq; i++)
            {
                if (strncasecmp(str, pattern[i], strlen(pattern[i])) == 0)
                {
                    foundPattern[i] = 1;
                    reValuePattern = i + 1;
                }
            }
            str++;
        }
    }

    return reValuePattern;
}

// --- Methode zum Zählen der Zeilen in einer Datei ---
int count_lines(const char *filename)
{
    FILE *file = fopen(filename, "r");
    checkFile(file);

    int count = 0;
    char ch;
    int lastCharWasNewline = 1;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            count++;
            lastCharWasNewline = 1;
        }
        else
        {
            lastCharWasNewline = 0;
        }
    }

    if (!lastCharWasNewline)
    {
        count++;
    }

    fclose(file);
    return count;
}

void errorCheck(int isError)
{
    if (isError)
        printf("\033[A\033[2K"); // Löschen der Error-Zeile
}

void outputFile(char *path)
{
    FILE *file = fopen(path, "r");
    checkFile(file);
    char ch;

    while ((ch = fgetc(file)) != EOF)
    {
        putchar(ch);
    }

    fclose(file);
}

void openFileReq(char *paths[], char *important_path)
{
    #ifdef _WIN32
    // --- Öffnen der finalen Dateien ---
    printf("Zum Öffnen der gewünschten finalen Log-Datei die jeweilige Taste drücken\n");
    char openFile;
    int getchSuccess = 0;
    int isError = 0;

    for (int i = 0; i < countPatternParamReq; i++)
    {
        printf("\033[36;1m%d\033[0m ... \033[32;1m'%s_log'\033[0m\n", i + 1, pattern[i]);
    }
    printf("\033[36;1mI\033[0m ... \033[32;1m'important_log'\033[0m\n");
    printf("Mit \033[36;1mENTER\033[0m fortfahren..\n\n");

    while (1)
    {
        if (getchSuccess)
        {
            break;
        }
        if (_kbhit())
        {
            errorCheck(isError);
            openFile = _getch();

            if (openFile == 13)
            {
                break;
            }
            char cmd[256];
            char befehl[256];

            for (int i = 0; i < countPatternParamReq; i++)
            {
                if (openFile == i + 1 + '0')
                {
                    getchSuccess = 1;
                    printf("\033[32;1m'%s_log'\033[0m wird geöffnet..\n\n", pattern[i]);
                    snprintf(cmd, sizeof(cmd), "start notepad \"%s\"", paths[i]);
                    system(cmd);
                }
            }

            if (openFile == 'I' || openFile == 'i')
            {
                getchSuccess = 1;
                printf("\033[32;1m'important_log'\033[0m wird geöffnet..\n\n");
                snprintf(befehl, sizeof(befehl), "start notepad \"%s\"", important_path);
                system(befehl);
            }
            else if (!getchSuccess)
            {
                printf("\033[31;1mUnbekannte\033[0m Taste gedrückt. \033[31;1mErneut versuchen..\033[0m\n");
                isError = 1;
            }
        }
    }
    #endif
}

void exportFilesReq(char dateTime[20])
{   
    #ifdef _WIN32
    printf("Wollen Sie ihre finalen Logs an einen anderen Speicherort exportieren?\n");
    printf("\033[36;1mY\033[0m ... Yes\n");
    printf("\033[36;1mN\033[0m ... No (Default) \n");

    char exportFile;
    int getchSuccess = 0;
    int isError = 0;

    while (1)
    {
        if (getchSuccess)
        {
            break;
        }
        if (_kbhit())
        {
            errorCheck(isError);
            exportFile = _getch();

            char befehl[1024];

            if (exportFile == 13)
            {
                getchSuccess = 1;
                break;
            }
            if (exportFile == 'Y' || exportFile == 'y')
            {
                char path[512];
                printf("Zu benutzender Export-Pfad: ");
                scanf("%511s", path);
                printf("\n");

                getchSuccess = 1;
                printf("\033[32;1m'Result'\033[0m wird exportiert..\n");

                snprintf(befehl, sizeof(befehl), "xcopy \"C:\\win\\SmartLog\\Results\\Result_%s\" \"%s\\SmartLog_Result_%s\" /E /I /H", dateTime, path, dateTime);
                system(befehl);
                snprintf(befehl, sizeof(befehl), "cp -rp /opt/SmartLog/Results/Result_%s %s/SmartLog_Result_%s", dateTime, path, dateTime);
                system(befehl);
            }
            else if (exportFile == 'N' || exportFile == 'n')
            {
                getchSuccess = 1;
                break;
            }
            else
            {
                printf("\033[31;1mUnbekannte\033[0m Taste gedrückt. \033[31;1mErneut versuchen..\033[0m\n");
                isError = 1;
            }
        }
    }
    #endif
}
int main(int argc, char *argv[])
{
    // --- Konsoleneinstellungen ---
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    system("cls");
#elif _linux_
    system("clear");
#endif

    if (!isNoOutputReq)
    {
        printf("\033[?25l");
        printf("\033[0m");
    }

    // --- File-Variablen ---
    char *iFilename;
    int debugging_mode = 0;
    int bot_mode = 0;
#ifdef _WIN32
    char *standard_file_path = "C:\\win\\SmartLog\\standard_log";
    char *emblem_file_path = "C:\\win\\SmartLog\\Emblem.txt";
    char *help_file_path = "C:\\win\\SmartLog\\help.txt";
#elif __linux__
    char *standard_file_path = "/opt/SmartLog/standard_log";
    char *emblem_file_path = "/opt/SmartLog/Emblem.txt";
    char *help_file_path = "/opt/SmartLog/help.txt";
#endif
    int countAccess = 0;

    // --- 1-Nano-Sekunde-Struct für Pause erstellt ---
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1;

    // --- Parameter-Überprüfung ---
    if (argc > 1)
    {
        int successParams = getParam(argc, argv);

        if (!successParams) // Falls Fehler bei Parameter-Bearbeitung, dann abbrechen
        {
            printf("\033[31;1mFehlerhafte Command-Syntax\033[0m\n\033[36;1m-help\033[0m oder \033[36;1m-?\033[0m für Hilfe\n");
            return 1;
        }

        if (isHelpParamReq) // Falls Hilfe-Parameter gesetzt, Hilfe ausgeben und abbrechen
        {
            outputFile(help_file_path);
            printf("\n");
            return 0;
        }

        if (isClearLogReq) // Falls Clear-Parameter gesetzt, werden alle Result-Logs gelöscht
        {
            char befehl[1024];

            if (!isNoOutputReq)
                printf("\033[32;1mLogs\033[0m wurden gelöscht..\n");

#ifdef _WIN32
            snprintf(befehl, sizeof(befehl), "rmdir /s /q \"C:\\win\\SmartLog\\Results\\\"");
            system(befehl);
            snprintf(befehl, sizeof(befehl), "mkdir \"C:\\win\\SmartLog\\Results\"");
            system(befehl);
#elif __linux__
            snprintf(befehl, sizeof(befehl), "rm -rf /opt/SmartLog/Results/");
            system(befehl);
            snprintf(befehl, sizeof(befehl), "mkdir /opt/SmartLog/Results");
            system(befehl);
#endif
            return 0;
        }

        if (isFileParamReq) // Falls File-Parameter gesetzt dann auch Variable auf diesen setzen
        {
            iFilename = paramFileName;
        }
        else // Wenn kein File-Parameter gesetzt, dann wird das Standard-File (access_log) als Input-File gesetzt
        {
            iFilename = standard_file_path;
            if (!isNoOutputReq)
                printf("Kein \033[36;1mFile-Parameter\033[0m => \033[32;1mStandard-File\033[0m wurde ausgewählt!\n");
        }

        if (isDebugParamReq) // Debugging Modus wird aktiviert
        {
            debugging_mode = 1;
        }

        if (countPatternParamReq < 1) // Wenn kein Pattern gewählt, dann wird Bot-Pattern ausgewählt
        {
            bot_mode = 1;
            pattern[0] = "bot";
            countPatternParamReq++;
        }
    }
    else // Wenn nur ein Parameter (Name), wird das Standard-File (acccess_log) als Input-File gesetzt
    {
        iFilename = standard_file_path;
        bot_mode = 1;
        pattern[0] = "bot";
        countPatternParamReq++;
        if (!isNoOutputReq)
            printf("Kein \033[36;1mFile-Parameter\033[0m => \033[32;1mStandard-File\033[0m wurde ausgewählt!\n");
    }

    // --- Emblem-Darstellung ---
    if (!isNoOutputReq)
    {
        outputFile(emblem_file_path);
    }
    // --- Öffnen der Files ---
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char dateTime[20];
    strftime(dateTime, sizeof(dateTime), "%Y%m%d_%H%M%S", t);

    char important_log_path[512];
    char *pattern_log_paths[countPatternParamReq];
    FILE *out_pattern_Files[countPatternParamReq];
    char command[1024];

#ifdef _WIN32
    snprintf(important_log_path, sizeof(important_log_path), "C:\\win\\SmartLog\\Results\\Result_%s\\important_log", dateTime);
    snprintf(command, sizeof(command), "mkdir \"C:\\win\\SmartLog\\Results\\Result_%s\"", dateTime);
    system(command);
#elif __linux__
    snprintf(important_log_path, sizeof(important_log_path), "/opt/SmartLog/Results/Result_%s/important_log", dateTime);
    snprintf(command, sizeof(command), "mkdir /opt/SmartLog/Results/Result_%s", dateTime);
    system(command);
#endif

    for (int i = 0; i < countPatternParamReq; i++)
    {
        char result_path[1024];
#ifdef _WIN32
        snprintf(result_path, sizeof(result_path), "C:\\win\\SmartLog\\Results\\Result_%s\\%s_log", dateTime, pattern[i]);
#elif __linux__
        snprintf(result_path, sizeof(result_path), "/opt/SmartLog/Results/Result_%s/%s_log", dateTime, pattern[i]);
#endif
        pattern_log_paths[i] = strdup(result_path);
        out_pattern_Files[i] = fopen(pattern_log_paths[i], "w");
        checkFile(out_pattern_Files[i]);
    }

    FILE *oFile = fopen(important_log_path, "w");
    FILE *iFile = fopen(iFilename, "r");
    int countFile = count_lines(iFilename);

    if (iFile == NULL)
    {
        fprintf(stderr, "\033[31;1mError\033[0m: Die Datei \033[32;1m'%s'\033[0m konnte nicht geöffnet werden.\n", iFilename);
        perror("\033[31;1mGrund\033[0m");
        return 1;
    }

    checkFile(oFile);

    if (!isNoOutputReq)
    {
        printf("Datei \033[32;1m'%s'\033[0m erfolgreich geöffnet.\n", iFilename);

        // --- Zähler auf der Console für bearbeitete Zugriffe ---
        printf("\nAnzahl identifizierter Zugriffe: \033[33;1m0\033[0m");
    }
#ifdef _WIN32
    Sleep(1000);
#elif __linux__
    sleep(1);
#endif
    fflush(stdout);

    char line[1024];

    while (fgets(line, sizeof(line), iFile))
    {
        int whichPattern = contains_pattern(line, bot_mode);

        if (whichPattern == 0)
        {
            fputs(line, oFile);
        }
        else
        {
            fputs(line, out_pattern_Files[whichPattern - 1]);
        }
        countAccess++;

        if (!isNoOutputReq)
        {
            if (countAccess % 100 == 0) // Jeden 100. Zugriffe ausgeben (kann geändert werden)
            {
                printf("\rAnzahl identifizierter Zugriffe: \033[33;1m%d\033[0m", countAccess);
                fflush(stdout);
                nanosleep(&ts, NULL);
            }
        }
    }

    if (!isNoOutputReq)
        printf("\rAnzahl identifizierter Zugriffe: \033[33;1m%d\033[0m\n", countFile); // Letzen Zugriff ausgeben

    fclose(iFile);
    fclose(oFile);

    // --- Ergebnis-Ausgabe ---
    if (!isNoOutputReq)
    {
        printf("\033[31;1;4m"
               "\nErgebnis:\n"
               "\033[0m");

        for (int i = 0; i < countPatternParamReq; i++)
        {
            fclose(out_pattern_Files[i]);

            printf("\033[33;1m"
                   "%d "
                   "\033[0m"
                   "Zugriffe wurden aufgrund des Wortes \033[36;1m'%s'\033[0m herausgefiltert\n",
                   count_lines(pattern_log_paths[i]), pattern[i]);
        }

        printf("\033[33;1m"
               "%d "
               "\033[0m"
               "Zugriffe wurden als bedeutend identifiziert \n\n",
               count_lines(important_log_path));
        #ifdef _WIN32 
        openFileReq(pattern_log_paths, important_log_path);
        exportFilesReq(dateTime);
        #endif
        printf("\nDanke für die Benutzung von SmartLog\n");

        // Cursor-Wiederherstellung
        printf("\033[?25h");
    }

    return 0;
}