#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits>
#include <algorithm>

using namespace std;

void displayPrompt(){
    char *user = getlogin(); //I assign the login name to a user pointer
    if(user == NULL){
        perror("getlogin");
        exit(1);
    }

    char hostname[100]; //I allocate space for the hostname
    if(gethostname(hostname, sizeof(hostname)) == -1){ //checking for errors withostname
        perror("gethostname");
        exit(1);
    }

    cout << user << '@' << hostname << '$' << ' '; //here I output the user and hostanme to the screen.

}


/*void splitSpaces(char* argument, char **pendingArgs, int pos){
    string temp(argument);
    string first_word, second_word;

    for(unsigned int i = 0; i < temp.size(); ++i){
        if( !(isspace(temp.at(i))) && (second_word.size() == 0) ){
            first_word += temp.at(i);
        }
        else if( (isspace(temp.at(i)))){
            if( !(isspace(temp.at(i+1))) ){
                second_word += temp.at(i+1);
                ++i;   
            }
        }
        else if( !(isspace(temp.at(i))) ){
            second_word += temp.at(i);
        }
    }

    cout << "THE FIRST PIECE: " << first_word << endl;
    pendingArgs[pos] = const_cast<char*>(first_word.c_str());
    
    pos++;
    
    cout << "THE SECOND PIECE: " << second_word << endl;
    pendingArgs[pos] = const_cast<char*>(second_word.c_str());

    pos++;
    pendingArgs[pos] = NULL; //YOUR ARGUMENT ARRAY MUST END IN A NULL PTR
    return;
}*/




void splitSpaces(char* argument, char **pendingArgs, int pos){
    char *token = strtok(argument, " ");
    while(token != NULL){
        cout << token << endl;
        token = strtok(NULL, " ");

        pendingArgs[pos] = token;
        pos++;

    }

    pendingArgs[pos] = NULL;
    cout << "position: " << pos << endl;
    cout << "arguments: " << pendingArgs[0] << ' ' << pendingArgs[1] << ' ' << pendingArgs[2];
    return;
}





int executeCommand(char **pendingArgs){
    int  pid = fork();

    if(pid == -1){
        perror("There was some error with fork()");
        exit(1);
    }
    else if(pid == 0){
        if(-1 == execvp(pendingArgs[0], pendingArgs)){
            perror("There was an error with execvp");
            exit(1);
        }
    }
    else if(pid > 0){
        if(wait(0) == -1){
            perror("There was some error with wait");
            exit(1);
        }
    }

    return 0; //if everything goes well, return 0 
}



int extractCommands(string &commandLine, char **cmds){
    const char* args = commandLine.c_str(); //the user input is converted to a const char* as we need it for execvp
    char* args_Mutatable = const_cast<char*>(args); //it needs to be mutable for us to tokenize it
    char *single_command;
    single_command = strtok(args_Mutatable, ";&|"); //execute strtok with delimeters
    
    int numOfArgs = 0; //when you add one --> argv[1]


    while(single_command != NULL){
        //cout << single_command << endl;
        cmds[numOfArgs] = strdup(single_command); //this processes each command into the command array for execution
        cout << "at position " << numOfArgs << ": " << cmds[numOfArgs];
        cout << endl;
        single_command = strtok(NULL, ";&|");
        numOfArgs++;
    }

    cmds[numOfArgs] = NULL;
    return numOfArgs;
}


vector<string>  extractConnectors(string inputLine){
    vector<string> cnctors;
    for(unsigned int i = 0; i < inputLine.size(); ++i){
        if(inputLine.at(i)  == ';'){
            cnctors.push_back(";");   
        }
        if((inputLine.at(i) == '&' && inputLine.at(i+1) == '&')){
            cnctors.push_back("&&");
        }
        if((inputLine.at(i) == '|' && inputLine.at(i+1) == '|')){
            cnctors.push_back("||");
        }
    }
    return cnctors;
}

bool noSpace(char* single){
    string temp(single);
    for(unsigned int i = 0; i < temp.size(); ++i){
        if(temp.at(i) == ' '){
            return false;   
        }
    }

    return true;
}

void run(vector<string> &connectors, char **cmds){
    char *pendingArgs[10000]; //arguments waiting to be executed depending on connector 
    if(connectors.size() == 0){
        if(noSpace(cmds[0])){
           pendingArgs[0] = cmds[0];
           pendingArgs[1] = NULL;
           executeCommand(pendingArgs);
           return;
        }
        else if(connectors.size() == 0 && sizeof(cmds[0]) >= 2){
            cout << "PASSES FIRST TEST" << endl;
            splitSpaces(cmds[0], pendingArgs, 0);
            executeCommand(pendingArgs);
            return;
        }
    }
    /*for(unsigned int i = 0; i < connectors.size(); ++i){
        if(connectors.at(i) == ";"){
            cout << "BEFORE: " << cmds[i] << endl;
            pendingArgs[i] = cmds[i];
            cout << "AFTER: " << pendingArgs[i] << endl;
            executeCommand(pendingArgs);
            cout << "REACHED3" << endl;
            splitSpaces(cmds[i], pendingArgs, i);
            cout << "REACHED4" << endl;
            cout << "First arg: " << pendingArgs[0]; 
            cout << "READED5" << endl;
            cout << "Second arg: " << pendingArgs[1];
            executeCommand(pendingArgs);
        }
    }*/
}


int main(){
    string userInput; //command line in string format
    char *cmds[10000];
    do{ 
        displayPrompt(); //displays current user and hostname

        getline(cin, userInput); //get's input from user and stores it in a string
 
        /*gets all the delimeters and stores into a vector*/
        vector<string> connectors = extractConnectors(userInput);
        cout << "CONNECTORS: ";
        for(unsigned i = 0; i < connectors.size(); ++i){
            cout << connectors.at(i) << ' ';
        }
        cout << endl;


        //cout << userInput << endl; //NEED TO DELETE UPON COMPLETETION


        int numArgs = extractCommands(userInput, cmds); //retrieve number of arguments by parsing the string


        if(numArgs <= 0){continue;} //if there are no arguments, simply continue to the next iteration

        /*checks if first argument is exit and quits iff it's the only argument*/
        if( (strcmp(cmds[0], "exit") == 0) && (numArgs == 1) ) { break; }
        
        run(connectors, cmds);

        /*if(executeCommand(cmds) != 0){
            cout << "Error in executing commands" << endl;
        }*/

    }while(1);
      

    return 0;
}
