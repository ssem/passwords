/////////////////////////////////////////////////////////////////////
//Note, this code was originally written by Matt Weir, at Florida State University
//Contact Info weir -at- cs.fsu.edu
//This code is licensed GPLv2
//If you make any changes I would love to hear about them so I don't have to impliment them myself ;)

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define MINWORDSIZE 3  //Minimum size for a word to match 
#define MAXWORDSIZE 14 //Maximum size for a word to match 
#define MAXLETTERS 26  //putting this in, just in case I decide to modify this for other languages
#define MAXPASSWORD 16 //Maximum size of a password to analyze
#define MAXPATTERN 400  //The Maximum size of a pattern string
#define MAXREPLACE 19  //The number of single character replacement options

typedef struct wordStruct {
  int isTerm;         //If this is an endpoint for a word
  wordStruct* next[MAXLETTERS];
}wordType;

typedef struct passStruct {
  char word[MAXWORDSIZE+1];  //the password to check
  int rule;                  //the rule that matches it.  Used for debugging
  passStruct* next;
}passType;

typedef struct patternStruct {
  char word[MAXPATTERN];  //the pattern
  int count;		     //the number of times the pattern is found
  patternStruct* next;
}patternType;


void sig_alrm(int signo)
{
  printf("exiting\n");
  exit(0);
}

int initilizeWordType(wordType *inputWords);
int readDic(wordType *inputWords,char *dicName);
int readPass(passType *inputWords,char *passName);
int findPos(char curChar);
int getNextWord(FILE *inputFile,char *nextWord);
int getNextWordRaw(FILE *inputFile,char *nextWord);
int parsePass(passType *passwords,wordType *inputWords);
int insertPattern(patternType **totalPatterns, char *curStruct);
int checkWord(wordType *inputWords, char *checkWord);
int convertLower(char *inputPass, int curPos);
int deleteChar(char *inputPass, int curPos);
int findNumInstances(char *inputPass, char checkChar);

//-------Word Editing Rules-----------------//
//--All rules return 1 if they change the word, 0 if they do not-//
int removePost(char *inputPass, char *pattern);
int removePre(char *inputPass, char *pattern);
int removeCase(char *inputPass, char *pattern);
int checkInsertChar(char *inputPass, char *pattern,int choice);
int replaceSingle(char *inputPass, char *pattern,int choice,int num);
int checkTwoWords(wordType *inputWords, char *checkWord, char *wordOne, char *wordTwo);

//The following is used to determine which letter is at which array location
//Note, findPos kind of takes over this functionality for lowercase letters
//Yah looking back I should have just used ASCII, duh I'm an idiot
char letters[MAXLETTERS+1]="abcdefghijklmnopqrstuvwxyz";
char upper[MAXLETTERS+1]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Used for substitution
char replaceListFrom[MAXREPLACE+2]="011234455@([{@|!iI$";  //the characters to replace
char replaceListWith[MAXREPLACE+2]="oliaehaszaceegiills";  //replace them with these

int main(int argc, char *argv[]) {
  wordType *inputWords;
  passType *passwords;
  passType *tempPass;
  struct sigaction action;
  char dicName[50];
  char passName[50];

  if (argc != 3) {
    printf("Usage: ./passPattern <dictionary> <password list>\n");
    exit(0);
  }

  strncpy(dicName,argv[1],49);
  strncpy(passName,argv[2],49);

//-----------------Signal Initilization--------------
  action.sa_handler = sig_alrm;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, 0);
  sigaction(SIGKILL, &action, 0);
  sigaction(SIGTERM, &action, 0);
  sigaction(SIGALRM, &action, 0);

//-----------------End Signal Initilization----------
  inputWords = new wordType;
  initilizeWordType(inputWords);
  if (readDic(inputWords,dicName)==-1) {
    printf("Error reading dictionary file\n");
    return -1;
  }
  printf("Finished pre-processing dictionary\n");
  passwords= new passType;
  if (readPass(passwords,passName)==-1) {
    printf("error reading password file\n");
    return -1;
  }
  if (passwords->next!=NULL) {
    tempPass=passwords;
    passwords=passwords->next;
    delete tempPass;
  }
  parsePass(passwords,inputWords);
  return 0;
}

int initilizeWordType(wordType *inputWords) {
  int i;
  inputWords->isTerm=0;
  for (i=0;i<MAXLETTERS;i++) {
    inputWords->next[i]=NULL;
  }
  return 0;
} 

int readDic(wordType *inputWords,char *dicName) {
  FILE *inputFile;
  char nextWord[MAXWORDSIZE+1];
  int done;
  char curChar;
  int i;
  wordType *curPos;
  wordType *tempWordType;
  int pos;
  
  inputFile=fopen(dicName,"r");
  if (inputFile==NULL) {
    return -1;
  }  
  done=getNextWord(inputFile,nextWord);
  while (done!=1) {
    if (done!=-1) {
      curPos=inputWords;
      for (i=0;i<MAXWORDSIZE;i++) {
        curChar=nextWord[i];
        if (curChar=='\0') {
          curPos->isTerm=1;
          break;
        }
        pos=findPos(curChar);
        if (pos!=-1) {
          if (curPos->next[pos]==NULL) {
            tempWordType= new wordType;
            initilizeWordType(tempWordType);
            curPos->next[pos]=tempWordType;
            curPos=tempWordType;
          }
          else {
            curPos=curPos->next[pos];
          }
        }
      }
    }
    done=getNextWord(inputFile,nextWord);
  }
  fclose(inputFile);
  return 0;
}

int readPass(passType *inputWords,char *passName) {
  FILE *inputFile;
  char nextWord[MAXWORDSIZE+1];
  int done;
  char curChar;
  passType *curPos;
  passType *tempWordType;

  inputFile=fopen(passName,"r");
  if (inputFile==NULL) {
    return -1;
  }

  done=getNextWordRaw(inputFile,nextWord);
  curPos=inputWords; 
  while (done!=1) {
    if (done!=-1) {
      tempWordType= new passType;
      strncpy(tempWordType->word,nextWord,MAXWORDSIZE+1);
      tempWordType->next=NULL;
      tempWordType->rule=0;
      curPos->next=tempWordType;
      curPos=tempWordType;
    }
    done=getNextWordRaw(inputFile,nextWord);
  }
  fclose(inputFile);
  return 0;
}



int findPos(char curChar) {
  if (curChar=='a') {
    return 0;
  }
  else if (curChar=='b') {
    return 1;
  }
  else if (curChar=='c') {
    return 2;
  }
  else if (curChar=='d') {
    return 3;
  }
  else if (curChar=='e') {
    return 4;
  }
  else if (curChar=='f') {
    return 5;
  }
  else if (curChar=='g') {
    return 6;
  }
  else if (curChar=='h') {
    return 7;
  }
  else if (curChar=='i') {
    return 8;
  }
  else if (curChar=='j') {
    return 9;
  }
  else if (curChar=='k') {
    return 10;
  }
  else if (curChar=='l') {
    return 11;
  }
  else if (curChar=='m') {
    return 12;
  }
  else if (curChar=='n') {
    return 13;
  }
  else if (curChar=='o') {
    return 14;
  }
  else if (curChar=='p') {
    return 15;
  }
  else if (curChar=='q') {
    return 16;
  }
  else if (curChar=='r') {
    return 17;
  }
  else if (curChar=='s') {
    return 18;
  }
  else if (curChar=='t') {
    return 19;
  }
  else if (curChar=='u') {
    return 20;
  }
  else if (curChar=='v') {
    return 21;
  }
  else if (curChar=='w') {
    return 22;
  }
  else if (curChar=='x') {
    return 23;
  }
  else if (curChar=='y') {
    return 24;
  }
  else if (curChar=='z') {
    return 25;
  }
  return -1;
}

//////////////////////////////////////////////////////////
//gets the next word from the inputfile
//only processes alpha characters
//converts all characters to lower case
//returns 0 if everything works ok
//returns 1 if end of file
//returns -1 if the word is not valid
int getNextWord(FILE *inputFile,char *nextWord) {
  char tempChar;
  int curCount=0;
  char tempString[2];
  tempChar=fgetc(inputFile);
  tempString[0]=tempChar;
  tempString[1]='\0';
//  if (convertLower(tempString,0)!=-1) {
//    tempChar=tempString[0];
//  }
  tolower(tempChar);
  printf("%c\n",tempChar);
  while(!feof(inputFile)&&(tempChar!='\n')&&(tempChar!='\r')) {
    if (strchr(letters,tempChar)==NULL) {
      return -1;
    }
    if (curCount < (MAXWORDSIZE)) {
      nextWord[curCount]=tempChar;
      curCount++;
    }
    tempChar=fgetc(inputFile);
    tempString[0]=tempChar;
//    if (convertLower(tempString,0)!=-1) {
//      tempChar=tempString[0];
//    }
    tolower(tempChar);
  }
  nextWord[curCount]='\0';  
  if (feof(inputFile)){
    return 1;
  }
  else if (curCount<MINWORDSIZE) {
    return -1;
  }
  else {
    return 0;
  }
} 

//////////////////////////////////////////////////////////
//gets the next word from the inputfile
//does not modify the words
//returns 0 if everything works ok
//returns 1 if end of file
//returns -1 if the word is not valid due to size issue
int getNextWordRaw(FILE *inputFile,char *nextWord) {
  char tempChar;
  int curCount=0;
  tempChar=fgetc(inputFile);
  while(!feof(inputFile)&&(tempChar!='\n')) {
    if (curCount < (MAXWORDSIZE)) {
      nextWord[curCount]=tempChar;
      curCount++;
    }
    tempChar=fgetc(inputFile);
  }
  nextWord[curCount]='\0';
  if (feof(inputFile)){
    return 1;
  }
  else if (curCount<MINWORDSIZE) {
    return -1;
  }
  else {
    return 0;
  }
}

//////////////////////////////////////////////////////////
//Parses the passwords against the input dictionary
int parsePass(passType *passwords,wordType *inputWords) {
  FILE *foundFile;
  FILE *notFoundFile;
  FILE *editFile;
  passType *tempPass;
  patternType *totalPatterns;
  patternType *tempPattern;
  char curWord[MAXWORDSIZE+1];
  char wordOne[MAXWORDSIZE+1];   //used to store the first word if two words are used
  char wordTwo[MAXWORDSIZE+1];   //used to store the second word if two words are used
  char curStruct[MAXPATTERN+1];
  totalPatterns=NULL; 
  int check;  
  int found;
  int i;
  int j;
  int foundCount;

  foundCount=0;
  foundFile=fopen("foundWords.txt","w");
  if (foundFile==NULL) {
    printf("ERROR can not open foundWords.txt for writing\n");
    return -1;
  }
  notFoundFile=fopen("unclassified.txt","w");
  if (notFoundFile==NULL) {
    printf("ERROR can not open unclassified.txt for writing\n");
    return -1;
  }
  editFile=fopen("editFile.txt","w");
  if (editFile==NULL) {
    printf("ERROR can not open editFile.txt for writing\n");
    return -1;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  //Start checking with the various rules
  tempPass=passwords;
  while (tempPass!=NULL) {
    found=0;
    if (tempPass->rule==0) { //this password hasn't been classified yet
      //----------------check basic password----------------------------------------------//
      strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
      strncpy(curStruct,"=",MAXPATTERN);
      found=checkWord(inputWords,curWord);  
      
      //----------------try two words instead of one--------------------------------------//
      if (found==0) {
        strncpy(curStruct,"TwoWords",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        found=checkTwoWords(inputWords,curWord,wordOne,wordTwo);
        if (found) {
//          printf("Two: Password=%s Word1=%s Word2=%s\n",curWord,wordOne,wordTwo);
        }
      }
      //----------------lowercase everything, removePre, removePost, then check-----------//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=0;
        check =removeCase(curWord,curStruct);
        if (check==1) {
          removePre(curWord,curStruct);
        }
        else {
          check =removePre(curWord,curStruct);
        }
        if (check==1) {
          removePost(curWord,curStruct);
        }
        else {
          check=removePost(curWord,curStruct);
        }
        if (check==1) {
          found=checkWord(inputWords,curWord);
        }
      }

      //--------------replace a single type of character, lowercase, removepre, removePost, then check-----//
      if (found==0) {
        for (i=0;i<MAXREPLACE;i++) {
          strncpy(curStruct,"",MAXPATTERN);
          strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
          j=1;
          //printf("Word=%s\n",curWord);
          check=replaceSingle(curWord, curStruct,i,j);
          while ((check!=0)&&(found==0)) {
            removeCase(curWord,curStruct);
            removePre(curWord,curStruct);
            removePost(curWord,curStruct);
            found=checkWord(inputWords,curWord);
            if (found!=0) {
     //         printf("orig=%s match=%s\n",tempPass->word,curWord);
            }
            else {
              j++;
              strncpy(curStruct,"",MAXPATTERN);
              strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
              check=replaceSingle(curWord,curStruct,i,j);
            }
          }
          if (found!=0) {
            break;
          }
        }
      }
      //--------------remove front character, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,1);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      //--------------remove last character, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,2);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      //--------------remove same front and end character, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,3);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      //--------------remove same front characters, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,5);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      //--------------remove same end characters, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,6);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      //--------------remove same front and end characters, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,7);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }

      //--------------remove different front and end character character, lowercase, removepre, removePost, then check---//
      if (found==0) {
        strncpy(curStruct,"",MAXPATTERN);
        strncpy(curWord,tempPass->word,MAXWORDSIZE+1);
        check=checkInsertChar(curWord,curStruct,4);
        if (check==1) {
          removeCase(curWord,curStruct);
          removePre(curWord,curStruct);
          removePost(curWord,curStruct);
          found=checkWord(inputWords,curWord);
          if (found!=0) {
//            printf("orig=%s match=%s\n",tempPass->word,curWord);
          }
        }
      }
      if (found!=0) {
        foundCount++;
        if (foundCount<101) printf("%i:%s -- %s\n",foundCount,tempPass->word,curWord);
        fprintf(foundFile,"%s\n",curWord);
        insertPattern(&totalPatterns,curStruct);
        tempPass->rule=found;
      }
    }
    tempPass=tempPass->next;
  }     
  printf("Total found =%i\n",foundCount);
  

  //////////////////////////////////////////////////
  //Write all the unclassified passwords to notFound
  tempPass=passwords;
  while (tempPass!=NULL) {
    passwords=passwords->next;
    if (tempPass->rule==0) {
      fprintf(notFoundFile,"%s\n",tempPass->word);
    }
    delete tempPass;
    tempPass=passwords;
  } 
  ////////////////////////////////////////////////
  //Writes the patterns
  tempPattern=totalPatterns;
  while (tempPattern!=NULL) {
    totalPatterns=totalPatterns->next;
    //printf("%s\n",tempPattern->word);
    fprintf(editFile,"%i\t%s\n",tempPattern->count,tempPattern->word);
    delete tempPattern;
    tempPattern=totalPatterns;
  } 
  return 0;
}


int insertPattern(patternType **totalPatterns, char *curStruct) {
  patternType *curPattern;
  patternType *newPattern;
  curPattern=*totalPatterns;
  if (curPattern!=NULL) {
    if (strcmp(curStruct, curPattern->word)==0) {
      curPattern->count++;
      return 0;
    }
    while (curPattern->next!=NULL) {
      if (strcmp(curStruct,curPattern->next->word)==0) {
        curPattern->next->count++;
        return 0;
      }
      curPattern=curPattern->next;
    }
  }
  //////////////////////////////////////
  //if it is a new pattern
  newPattern = new patternType;
  newPattern->count=1;
  strncpy(newPattern->word,curStruct,MAXPATTERN);
  newPattern->next = NULL;
  if (curPattern!=NULL) {
    curPattern->next=newPattern;
  }
  else {
    *totalPatterns = newPattern;
  }
  return 0;
}



///////////////////////////////////////////////////////////////
//Checks to see if checkWord is in the dictionary inputWords
//returns 0 if false
//returns 1 if true
int checkWord(wordType *inputWords, char *checkWord) {
  int i;
  int wordSize;
  int curPos;
  wordType *tempWord;
  if (checkWord==NULL) {
    return 0;
  }
  wordSize=strlen(checkWord);  //don't care about the '\0' character

  tempWord=inputWords;
  for (i=0;i<wordSize;i++) {
    curPos=findPos(checkWord[i]);
    if (curPos==-1) {
      return 0;
    }
    if (tempWord->next[curPos]==NULL) {  //Not in the input dictionary
      return 0;
    }
    tempWord=tempWord->next[curPos];
  }
  if (tempWord->isTerm==1) {  //if it is at the end of a valid word
    return 1;
  }
  return 0;
}


////////////////////////////////////////////////////////////////
//Checks to see if checkword is composed of two words in the input dictionary
//returns 0 if false
//returns 1 if true
int checkTwoWords(wordType *inputWords, char *checkWord, char *wordOne, char *wordTwo) {
  int i,j;
  int wordSize; //the size of the word being checked
  int curPos;   //Used to tell where in the tree to check for a letter
  int sizeOne;  //the current size of the first word
  int sizeTwo;  //the current size of the second word
  int backup;
  int isFirst;
  wordType *tempWord;
  wordType *backupPos;   //used to go back to try other possible words for the first one
  sizeOne=0;
  sizeTwo=0;
  backup=0;
  isFirst=1;
  if (checkWord==NULL) {
    return 0;
  }
  wordSize=strlen(checkWord);
  if (wordSize>MAXWORDSIZE) {
    return 0;
  }
  tempWord=inputWords;
  for (i=0;i<wordSize;i++) {
    curPos=findPos(checkWord[i]);
//    printf("Round=%i letter=%c\n",i,checkWord[i]);
    if (curPos==-1) { //if it is an invalid character
      return 0;
    }
    if ((tempWord->next[curPos]==NULL)&&(!(isFirst&&(tempWord->isTerm)))) {  //Not in the input dictionary
      if (isFirst) { //Can't find a valid first word
//        printf("isFirst and curPos=NULL, curPos=%c\n",checkWord[i]);
        return 0;
      }
      else {   //the second word didn't pan out   
//        printf("second word didn't work, restarting first\n backup word=");
        tempWord=inputWords; //continue the first word if possible
        isFirst=1;
        //sizeOne;
        i=sizeOne-1;  //have to reset the counter
        for (j=0;j<=i;j++) {
//          printf("%c",checkWord[j]);
          curPos=findPos(checkWord[j]);
          tempWord=tempWord->next[curPos];
        }
        wordOne[sizeOne-1]=checkWord[sizeOne-1];
        wordOne[sizeOne]='\0';
//        printf("wordOne looks like at this point %s\n", wordOne);
        if (tempWord==NULL) {
          return 0;
        }
//        printf("\n next letter = %c\n", checkWord[sizeOne]);
      }
    }
    else if ((isFirst)&&(tempWord->isTerm==1)&&(i>=2)) { //start checking the second word
//      printf("moving to second word\n");
      isFirst=0;
      backup=sizeOne;
      backupPos=tempWord;
      sizeTwo=-1;  //it's easier to set it to -1 so when it's incremented at the end it is 0
      tempWord=inputWords;  //reset tempWord for the 2nd word
      wordOne[sizeOne]='\0';
      i--;                             //gotta back up one position
    }
    else {
      if (isFirst) {
//        printf("working on first\n");
        wordOne[i]=checkWord[i];
        wordOne[i+1]='\0';
//        printf("wordOne looks like %s\n",wordOne);
      }
      else {
//        printf("working on second\n");
        wordTwo[sizeTwo]=checkWord[i];
      }
      tempWord=tempWord->next[curPos];
    }
    if (isFirst) {
      sizeOne++;
    }
    else {
      sizeTwo++;
    }
  }
  if ((!isFirst)&&(tempWord->isTerm==1)) {   //there are two valid words
    wordTwo[sizeTwo]='\0';
    return 1;
  }
  else {
    return 0;
  }
}     
        

////////////////////////////////////////////////////////////////
//Removes nonalpha characters from the end of a password
//returns 1 if changes were made
//returns 0 if no changes were made
int removePost(char *inputPass, char *pattern) {
  char tempChar;
  char tempPattern[MAXPATTERN];
  char tempPattern2[MAXPATTERN];
  int numStrip;
  int size;
  int curPos;
  int status;  //0=special character, 1=number, -1=not initilized
  int curCount;  //Used to tell how many numbers/special characters in the current run

  size=strlen(inputPass);
  curPos=size-1;
  numStrip=0;
  status=-1;
  curCount=0;

  strncpy(tempPattern,"",MAXPATTERN);
  while ((curPos>=MINWORDSIZE)&&(findPos(inputPass[curPos])==-1)) {
    numStrip++;
    if (isdigit(inputPass[curPos])!=0) {  //it's a number
      if ((status==-1)||(status==1)){
        curCount++;
        status=1;
      }
      else { //the previous one was a special character
        snprintf(tempPattern2,MAXPATTERN,"S%i%s",curCount,tempPattern);
        strncpy(tempPattern,tempPattern2,MAXPATTERN);
        curCount=1;
        status=1;
      }
    }
    else {  //it's not a number
      if ((status==-1)||(status==0)){
        curCount++;
        status=0;
      }
      else { //the previous one was a number
        snprintf(tempPattern2,MAXPATTERN,"N%i%s",curCount,tempPattern);
        strncpy(tempPattern,tempPattern2,MAXPATTERN);
        curCount=1;
        status=0;
      }
    }
    curPos--;
  }
  if (curPos<MINWORDSIZE) {
    return 0;
  }
  else if (numStrip==0) {
    return 0;
  }
  else if (numStrip!=0) {
    inputPass[curPos+1]='\0';
    if (status==0) {
      snprintf(tempPattern2,MAXPATTERN,"Post:S%i%s ",curCount,tempPattern);
      strncpy(tempPattern,tempPattern2,MAXPATTERN);
    }
    else {
      snprintf(tempPattern2,MAXPATTERN,"Post:N%i%s ",curCount,tempPattern);
      strncpy(tempPattern,tempPattern2,MAXPATTERN);
    }
    strncat(pattern,tempPattern,MAXPATTERN);
  }
  return 1;
}


////////////////////////////////////////////////////////////////
//Removes nonalpha characters from the begining of a password
//returns 1 if changes were made
//returns 0 if no changes were made
int removePre(char *inputPass, char *pattern) {
  char tempChar;
  char tempPattern[MAXPATTERN];
  char tempPattern2[MAXPATTERN];
  char tempWord[MAXWORDSIZE+1];
  int numStrip;
  int curPos;
  int status;  //0=special character, 1=number, -1=not initilized
  int curCount;  //Used to tell how many numbers/special characters in the current run

  curPos=0;
  numStrip=0;
  status=-1;
  curCount=0;
  

  strncpy(tempPattern,"Pre:",MAXPATTERN);
  while ((curPos<=MAXWORDSIZE)&&(findPos(inputPass[curPos])==-1)&&(inputPass[curPos]!='\0')) {
    numStrip++;
    if (isdigit(inputPass[curPos])!=0) {  //it's a number
      if ((status==-1)||(status==1)){
        curCount++;
        status=1;
      }
      else { //the previous one was a special character
        snprintf(tempPattern2,MAXPATTERN,"%sS%i",tempPattern,curCount);
        strncpy(tempPattern,tempPattern2,MAXPATTERN);
        curCount=1;
        status=1;
      }
    }
    else {  //it's not a number
      if ((status==-1)||(status==0)){
        curCount++;
        status=0;
      }
      else { //the previous one was a number
        snprintf(tempPattern2,MAXPATTERN,"%sN%i",tempPattern,curCount);
        strncpy(tempPattern,tempPattern2,MAXPATTERN);
        curCount=1;
        status=0;
      }
    }
    curPos++;
  }
  if (curPos>MAXWORDSIZE) {
    return 0;
  }
  else if (curPos==0) {
    return 0;
  }
  else if (curPos>0) {
    strncpy(tempWord,inputPass+curPos,MAXWORDSIZE);
    strncpy(inputPass,tempWord,MAXWORDSIZE);
    if (strlen(inputPass)<MINWORDSIZE) {
      return 0;
    }
    if (status==0) {
      snprintf(tempPattern2,MAXPATTERN,"%sS%i ",tempPattern,curCount);
      strncpy(tempPattern,tempPattern2,MAXPATTERN);
    }
    else {
      snprintf(tempPattern2,MAXPATTERN,"%sN%i ",tempPattern,curCount);
      strncpy(tempPattern,tempPattern2,MAXPATTERN);
    }
    strncat(pattern,tempPattern,MAXPATTERN);
  }
  return 1;
}


////////////////////////////////////////////////////////////////
//Makes the password lowercase.
//Note, only lowercases the first wordgroup
//It is recommended that you call this after character replacement
//returns 1 if changes were made
//returns 0 if no changes were made
int removeCase(char *inputPass, char *pattern) {
  char tempPattern[MAXPATTERN];
  int curPos;
  int status;  //uppercase status 0=first character, 1=last character, ,2=all uppercase, 3=only first lower, 4=only last lower, 5==mixed case, 6=no upper
  int foundUpper;  //used to tell if a change was made or not
 
  curPos=0;
  status=-1;
  foundUpper=0;

  strncpy(tempPattern,"Case:",MAXPATTERN);

  //Goes to the first letters
  while ((curPos<=MAXWORDSIZE)&&(inputPass[curPos]!='\0')&&((inputPass[curPos])==-1)&&(strchr(upper,inputPass[curPos])==NULL)) {
    curPos++;
  }
  if ((curPos==MAXWORDSIZE)||(inputPass[curPos]=='\0')) {
    return 0;
  }
  //Ready to start processing letters
  //Since the first character is special as far as the grammar goes, process it seperatly
  if (strchr(upper,inputPass[curPos])!=NULL) {
    status=0;
    convertLower(inputPass,curPos);
    foundUpper=1;
  }
  else {
    status=3;
  }
  curPos++;
  //Process the rest of the characters
  while ((curPos<=MAXWORDSIZE)&&(inputPass[curPos]!='\0')&&(((inputPass[curPos])!=-1)||(strchr(upper,inputPass[curPos])!=NULL))) {
    if (strchr(upper,inputPass[curPos])!=NULL) {
      foundUpper=1;
      convertLower(inputPass,curPos);
      if (status==0) {
        status=2;
      }
      else if (status==1) {
        status=5;
      }
      else if (status==4) {
        status=5;
      }
      else if (status==6) {
        status=1;
      }
    }
    else {
      if (status==1) {
        status=5;
      }
      else if (status==2) {
        status=4;
      } 
      else if ((status==3)&&(foundUpper==0)) {
        status=6;
      }
      else if (status==3) {
        status=5;
      }    
      else if (status==4) {
        status=5;
      }
    }
    curPos++;
  }
  //if no change was made
  if (foundUpper!=1) {   
    return 0;
  }
  if (status==0) {
    strncat(tempPattern,"First_Upper",MAXPATTERN);
  }
  else if (status==1) {
    strncat(tempPattern,"Last_Upper",MAXPATTERN);
  }
  else if (status==2) {
    strncat(tempPattern,"All_Upper",MAXPATTERN);
  }
  else if (status==3) {
    strncat(tempPattern,"First_Lower_Rest_Upper",MAXPATTERN);
  }
  else if (status==4) {
    strncat(tempPattern,"Last_Lower_Rest_Upper",MAXPATTERN);
  }
  else if (status==5) {
    strncat(tempPattern,"Mixed_Case",MAXPATTERN);
  }
  else {
    strncat(tempPattern,"Error",MAXPATTERN);
    printf("****************************************STATUS=%i\n",status);
  }
  strncat(pattern,tempPattern,MAXPATTERN);
  strncat(pattern," ",MAXPATTERN);
  return 1;
}


///////////////////////////////////////////////////////////////
//converts 1 character to lowercase.
//Converts it at curPos
//No checking, so make sure curPos is a valid address
//This function was written instead of using toupper so we can support international langauges
//In retrospect I could have used toupper to deal with English characters though ;(
int convertLower(char *inputPass, int curPos) {
  if (inputPass[curPos]=='A') {
    inputPass[curPos]='a';
    return 0;
  }
  if (inputPass[curPos]=='B') {
    inputPass[curPos]='B';
    return 0;
  }
  if (inputPass[curPos]=='C') {
    inputPass[curPos]='c';
    return 0;
  }
  if (inputPass[curPos]=='D') {
    inputPass[curPos]='d';
    return 0;
  }
  if (inputPass[curPos]=='E') {
    inputPass[curPos]='e';
    return 0;
  }
  if (inputPass[curPos]=='F') {
    inputPass[curPos]='f';
    return 0;
  }
  if (inputPass[curPos]=='G') {
    inputPass[curPos]='g';
    return 0;
  }
  if (inputPass[curPos]=='H') {
    inputPass[curPos]='h';
    return 0;
  }
  if (inputPass[curPos]=='I') {
    inputPass[curPos]='i';
    return 0;
  }
  if (inputPass[curPos]=='J') {
    inputPass[curPos]='j';
    return 0;
  }
  if (inputPass[curPos]=='K') {
    inputPass[curPos]='k';
    return 0;
  }
  if (inputPass[curPos]=='L') {
    inputPass[curPos]='l';
    return 0;
  }
  if (inputPass[curPos]=='M') {
    inputPass[curPos]='m';
    return 0;
  }
  if (inputPass[curPos]=='N') {
    inputPass[curPos]='n';
    return 0;
  }
  if (inputPass[curPos]=='O') {
    inputPass[curPos]='o';
    return 0;
  }
  if (inputPass[curPos]=='P') {
    inputPass[curPos]='p';
    return 0;
  }
  if (inputPass[curPos]=='Q') {
    inputPass[curPos]='q';
    return 0;
  }
  if (inputPass[curPos]=='R') {
    inputPass[curPos]='r';
    return 0;
  }
  if (inputPass[curPos]=='S') {
    inputPass[curPos]='s';
    return 0;
  }
  if (inputPass[curPos]=='T') {
    inputPass[curPos]='t';
    return 0;
  }
  if (inputPass[curPos]=='U') {
    inputPass[curPos]='u';
    return 0;
  }
  if (inputPass[curPos]=='V') {
    inputPass[curPos]='v';
    return 0;
  }
  if (inputPass[curPos]=='W') {
    inputPass[curPos]='w';
    return 0;
  }
  if (inputPass[curPos]=='X') {
    inputPass[curPos]='x';
    return 0;
  }
  if (inputPass[curPos]=='Y') {
    inputPass[curPos]='y';
    return 0;
  }
  if (inputPass[curPos]=='Z') {
    inputPass[curPos]='z';
    return 0;
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////
//Removes alpha characters from the password
//Switch tells it the behavior to use
//1=delete from the start of the string
//2=delete from the end of the string
//3=delete from both the start and end of string if both are the same
//4=delete from both the start and end of the string even if different
//5=delete multiple instances if same from the front
//6=delete multiple instances if same from the end
//7=delete multiple instances if same from the front and end
//Note, was written to be applied before case mangling rules but it doesn't matter
//You might just get the wrong impression on which characters to delete if you do case mangling first
//Aka, delete b instead of delete B
int checkInsertChar(char *inputPass, char *pattern,int choice) {
  char tempPattern[MAXPATTERN];
  int curPos;
  char curDelete;
  char saveChar;
  int status;
 
  curPos=0;
  status=-1;
  char tempWord[2];
  
  tempWord[1]='\0';
  //Goes to the first letters
  while ((curPos<=MAXWORDSIZE)&&((inputPass[curPos])==-1)&&(strchr(upper,inputPass[curPos])==NULL)&&(inputPass[curPos]!='\0')) {
    curPos++;
  }
  if ((curPos==MAXWORDSIZE)||(inputPass[curPos]=='\0')) {
    return 0;
  }
  curDelete=inputPass[curPos];
  saveChar=curDelete;

  if ((choice==1)||(choice==3)||(choice==4)||(choice==5)||(choice==7)) {
    deleteChar(inputPass,curPos);
  }
  if ((choice==1)||(choice==4)) {
    snprintf(tempPattern,MAXPATTERN,"Insert:(front:%c) ",curDelete);
  }
  else if (choice==3) {
    snprintf(tempPattern,MAXPATTERN,"Insert:(front+end:%c) ",curDelete);
  }
  else if (choice==5) {
    snprintf(tempPattern,MAXPATTERN,"Insert:(front:%c",curDelete);
  }
  else if (choice==7) {
    snprintf(tempPattern,MAXPATTERN,"Insert:(front+end%c",curDelete);
  }
  if (choice==1) {
    strncat(pattern,tempPattern,MAXPATTERN);
    return 1;
  }
  if ((choice==5)||(choice==7)) {
    tempWord[0]=curDelete;
    while (inputPass[curPos]==curDelete) {
      deleteChar(inputPass,curPos);
      strncat(tempPattern,tempWord,MAXPATTERN);
      status=1;
    }
  }
  if ((status==1)&&(choice==5)) {
    strncat(tempPattern,")",MAXPATTERN);
    strncat(pattern,tempPattern,MAXPATTERN);      
    return 1;
  }
  else if (choice==5) {  //There weren't any continuous strings of the same character
    return 0;
  }

  ////Go to the end of the string
  while ((curPos<=MAXWORDSIZE)&&((findPos(inputPass[curPos])!=-1)||(strchr(upper,inputPass[curPos])!=NULL))&&(inputPass[curPos]!='\0')) {
    curPos++;
  }
  if (curPos>0) {
    curPos--;
  } 
  else {
    return 0;
  }
  curDelete=inputPass[curPos];
  deleteChar(inputPass,curPos);
  if (curPos>0) {
    curPos--;
  }
  else {
    return 0;
  }
  if ((choice==2)||(choice==4)) {
    snprintf(tempPattern,MAXPATTERN,"Insert:(end:%c) ",curDelete);
    strncat(pattern,tempPattern,MAXPATTERN);
    return 1;
  }
  else if (choice==3) {
    if (curDelete==saveChar) {
      strncat(pattern,tempPattern,MAXPATTERN);
      return 1;
    }
    return 0;
  }
  else if ((choice==6)||(choice==7)) {
    if (choice==6) {
      snprintf(tempPattern,MAXPATTERN,"Insert:(end%c",curDelete);
    }
    else if (choice==7) {
      if (curDelete!=saveChar) {
        return 0;
      }
    }
    tempWord[0]=curDelete;
    while (inputPass[curPos]==curDelete) {
      deleteChar(inputPass,curPos);
      if (curPos>0) {
        curPos--;
      }
      else {
        return 0;
      }
      status=1;
      strncat(tempPattern,tempWord,MAXPATTERN);
    }
    if ((choice==6)||(choice==7)) {  
      if (status!=1) {
        return 0;
      }
      else {
        strncat(tempPattern,") ",MAXPATTERN);
        strncat(pattern,tempPattern,MAXPATTERN);
        return 1;
      }
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//Deletes the character at curPos
int deleteChar(char *inputPass, int curPos) {
  char tempChar;
  if (inputPass==NULL) {
    return -1;
  }
  if (strlen(inputPass)<=curPos) {
    printf("hey, check this out, delete char is trying to delete a char that doesn't exist\n");
    return -1;
  }
  tempChar=inputPass[curPos+1];
  while (tempChar!='\0') {
    inputPass[curPos]=tempChar;
    curPos++;
    tempChar=inputPass[curPos+1];
  }
  inputPass[curPos]='\0';
  return 0;
}

////////////////////////////////////////////////////////////////////////
//Returns the number of instances of a character in a password
int findNumInstances(char *inputPass, char checkChar) {
  int count;
  int curPos;

  count=0;
  curPos=0;
  if (inputPass==NULL) {
    return 0;
  }
  while (inputPass[curPos]!='\0') {
    if (inputPass[curPos]==checkChar) {
      count++;
    }
    curPos++;
  }
  return count;
}


////////////////////////////////////////////////////////////////////////////
//Replaces up to num characters in the password
//Goes from right to left
int replaceSingle(char *inputPass, char *pattern,int choice,int num) {
  char tempPattern[MAXPATTERN];
  int curPos;
  int curNum;
  char checkChar;

  curNum=0;
  curPos=0;
  if (choice>MAXREPLACE) {
    return 0;
  }
  checkChar=replaceListFrom[choice];
  if (inputPass==NULL) {
    return 0;
  }
  while ((curNum<=num)&&(curPos<=MAXWORDSIZE)&&(inputPass[curPos]!='\0')) {
    if (inputPass[curPos]==checkChar) {
      curNum++;
      inputPass[curPos]=replaceListWith[choice];
    }
    curPos++;
  }
  
  if (num==curNum) {
    snprintf(tempPattern,MAXPATTERN,"Replace:(%i %c:%c) ",num,checkChar,replaceListWith[choice]);
    strncat(pattern,tempPattern,MAXPATTERN);
    return 1;
  }     
  return 0;
}
