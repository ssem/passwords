#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <list>

using namespace std;

typedef struct passStruct {
  wstring plainValue;
  int size;
  int complexity;
}plainType;

typedef struct freqStruct {
  wchar_t value;
  int count;
}freqType;


bool getNextRecord(wifstream *inputFile,wstring *plainValue);
bool comparePlainType(plainType first, plainType second);
int findBasicComplexity(wstring plainValue);
bool isLowerChar(wchar_t inputChar);
bool isUpperChar(wchar_t inputChar);
bool isDigitChar(wchar_t inputChar);
bool isSpecialChar(wchar_t inputChar);
bool isNotEnglish(wchar_t inputChar);
bool measureOverallCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber);
bool measureFirstCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber);
bool measureLastCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber);
bool isComplexPassword(wstring inputString);
int hasUpperCase(wstring inputString);
int hasSpecial(wstring inputString);
int hasDigit(wstring inputString);
int onlyLowerCase(wstring inputString);

bool sortFreqFunction(freqType first, freqType second);

int main(int argc, char *argv[]) {
  wifstream inputFile;                   //the input file
  wofstream outputFile;                  //outputs the results to this file
  wofstream complexPasswordsFile;         //saves all the "strong" passwords to this file
  wstring plainValue;                    //used to pass the plaintext guess to the different functions
  plainType insertValue;                 //used to store password info for later use
  int totalPasswords=0;                  //the total number of passwords read in from the input file
  int totalSizeCountingUncracked=0;      //the total number of passwords in the target set
  int averageSizeRunningValue=0;           //the current running count of the average size
  int averageComplexityValue=0;
  string inputFileName;                  //the input file name
  int commandLineNum=1;                  //used to parse the command line input
  long overallCharFrequencyAnalysis[256];  //used to store the number of characters encountered, ASCII values, values 0-127
                                          //yes I know values 0-21 will never be used. 128-255 are used for non-english characters
  long firstCharFrequencyAnalysis[256];
  long lastCharFrequencyAnalysis[256];
  wstring nonEnglish_Chars;              //used to store the non-english characters that are encountered
  long numberOfCharactersOverall=0;        //stores the total number of characters in the password file
  long numberOfCharactersFirst=0;
  long numberOfCharactersLast=0;
  long numberOfPasswordsWithUpper=0;
  long numberOfPasswordsWithSpecial=0;
  long numberOfPasswordsWithDigit=0;
  long numberOfPasswordsOnlyLower=0;  

  list <freqType>::iterator it;
  if (argc < 3){
    printf("Usage: ./pass_stat -file <plain list of cracked passwords>\n");
    printf("Other options: -totalPasswords <total number of passwords including uncracked passwords>\n");
    return 0;
  }
  while (commandLineNum<argc) {
    string tempValue;
    tempValue = argv[commandLineNum];
    if (tempValue.compare("-file")==0) {
      commandLineNum++;
      if (commandLineNum==argc) {
        printf("You need to specify an input file to parse\n");
        return 0;
      }
  
      inputFileName=argv[commandLineNum];
    }
    else if (tempValue.compare("-totalPasswords")==0) {
      commandLineNum++;
      if (commandLineNum==argc) {
        printf("You need to specify the total passwords, (including uncracked passwords), to use this option\n");
        return 0;
      }
      totalSizeCountingUncracked=atoi(argv[commandLineNum]);
    }
    else {
      printf("Unknown option specified\n");
      return 0;
    }
    commandLineNum++;
  }
  if (inputFileName.empty()) {
    printf("You need to enter a filename\n");
    return 0;
  }     
  inputFile.open(inputFileName.c_str());
  if (!inputFile.is_open()) {
    printf("ERROR input file NOT FOUND\n");
    return 1;
  }
  outputFile.open("results.txt");
  if (!outputFile.is_open()) {
    printf("ERROR could not open results.txt for writing\n");
    return 1;
  }
  complexPasswordsFile.open("complex_passwords.txt");
  if (!complexPasswordsFile.is_open()) {
    printf("ERROR could not open complex_passwords.txt for writing\n");
    return 1;
  }
  for (int i=0; i<256; i++) {
    overallCharFrequencyAnalysis[i]=0;
    firstCharFrequencyAnalysis[i]=0;
    lastCharFrequencyAnalysis[i]=0;
  }
  

  while (getNextRecord(&inputFile,&plainValue)) {
    insertValue.plainValue=plainValue;
    insertValue.size = plainValue.size();
    totalPasswords++;
    averageSizeRunningValue=averageSizeRunningValue + insertValue.size;
    insertValue.complexity = findBasicComplexity(plainValue);
    averageComplexityValue = averageComplexityValue + findBasicComplexity(plainValue);
    measureOverallCharFreq(plainValue, overallCharFrequencyAnalysis, &nonEnglish_Chars, &numberOfCharactersOverall);
    measureFirstCharFreq(plainValue,firstCharFrequencyAnalysis,&nonEnglish_Chars,&numberOfCharactersFirst);
    measureLastCharFreq(plainValue,lastCharFrequencyAnalysis,&nonEnglish_Chars,&numberOfCharactersLast);    
    numberOfPasswordsWithUpper=numberOfPasswordsWithUpper + hasUpperCase(plainValue);
    numberOfPasswordsWithSpecial=numberOfPasswordsWithSpecial + hasSpecial(plainValue);
    numberOfPasswordsWithDigit=numberOfPasswordsWithDigit + hasDigit(plainValue);
    numberOfPasswordsOnlyLower=numberOfPasswordsOnlyLower + onlyLowerCase(plainValue);
    if (isComplexPassword(plainValue)) {
      complexPasswordsFile << plainValue << endl;
    }
    
  }
  cout << endl<<endl;
  if (totalSizeCountingUncracked!=0) {
    cout << "Total Number of Passwords:                   " << totalSizeCountingUncracked << endl;
    outputFile << "Total Number of Passwords:                   " << totalSizeCountingUncracked << endl;
  }
  cout << "Total Plaintext Values Parsed:               " << totalPasswords << "\n";
  cout << "Average Password Length of Parsed Passwords: " << averageSizeRunningValue/(totalPasswords*1.0) << endl;
  outputFile << "Total Plaintext Values Parsed:               " << totalPasswords << "\n";
  outputFile << "Average Password Length of Parsed Passwords: " << averageSizeRunningValue/(totalPasswords*1.0) << endl;

  if (totalSizeCountingUncracked!=0) {
    cout << "If the Average Length of Uncracked Passwords = 7, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*7)/(totalSizeCountingUncracked*1.0) << endl;
    cout << "If the Average Length of Uncracked Passwords = 8, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*8)/(totalSizeCountingUncracked*1.0) << endl;
    cout << "If the Average Length of Uncracked Passwords = 9, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*9)/(totalSizeCountingUncracked*1.0) << endl;
    outputFile << "If the Average Length of Uncracked Passwords = 7, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*7)/(totalSizeCountingUncracked*1.0) << endl;
    outputFile << "If the Average Length of Uncracked Passwords = 8, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*8)/(totalSizeCountingUncracked*1.0) << endl;
    outputFile << "If the Average Length of Uncracked Passwords = 9, Average Length: " << (averageSizeRunningValue+(totalSizeCountingUncracked-totalPasswords)*9)/(totalSizeCountingUncracked*1.0) << endl;


  }
  cout << "Average Complexity Level of Parsed Passwords: " << averageComplexityValue/(totalPasswords*1.0) << endl;
  outputFile << "Average Complexity Level of Parsed Passwords: " << averageComplexityValue/(totalPasswords*1.0) << endl;
  if (totalSizeCountingUncracked!=0) {
    cout << "If the Average Complexity of Uncracked Passwords = 3, Average Complexity: " << (averageComplexityValue+(totalSizeCountingUncracked-totalPasswords)*3)/(totalSizeCountingUncracked*1.0) << endl;
    outputFile << "If the Average Complexity of Uncracked Passwords = 3, Average Complexity: " << (averageComplexityValue+(totalSizeCountingUncracked-totalPasswords)*3)/(totalSizeCountingUncracked*1.0) << endl;

  }
  if (totalSizeCountingUncracked!=0) {
    cout << "If the Average Complexity of Uncracked Passwords = 4, Average Complexity: " << (averageComplexityValue+(totalSizeCountingUncracked-totalPasswords)*4)/(totalSizeCountingUncracked*1.0) << endl;
    outputFile << "If the Average Complexity of Uncracked Passwords = 4, Average Complexity: " << (averageComplexityValue+(totalSizeCountingUncracked-totalPasswords)*4)/(totalSizeCountingUncracked*1.0) << endl;
  }
  cout << "Percentage of Passwords that have an uppercase character: " << numberOfPasswordsWithUpper/(totalPasswords*1.0) << endl;
  cout << "Percentage of Passwords that have a special character:   " << numberOfPasswordsWithSpecial/(totalPasswords*1.0) << endl;
  cout << "Percentage of Passwords that have a digit:               " << numberOfPasswordsWithDigit/(totalPasswords*1.0) << endl;  
  cout << "Percentage of Passwords that only have lower characters:  " << numberOfPasswordsOnlyLower/(totalPasswords*1.0) << endl;

  outputFile << "Percentage of Passwords that have an uppercase character: " << numberOfPasswordsWithUpper/(totalPasswords*1.0) << endl;
  outputFile << "Percentage of Passwords that have a special character:   " << numberOfPasswordsWithSpecial/(totalPasswords*1.0) << endl;
  outputFile << "Percentage of Passwords that have a digit:               " << numberOfPasswordsWithDigit/(totalPasswords*1.0) << endl;
  outputFile << "Percentage of Passwords that only have lower characters:  " << numberOfPasswordsOnlyLower/(totalPasswords*1.0) << endl;
  outputFile << endl << endl;


  list <freqType> sortedOverallFreq;
  list <freqType> sortedFirstFreq;
  list <freqType> sortedLastFreq;
  freqType tempValue;
  for (int i=32;i<127+nonEnglish_Chars.size(); i++) {
    if (i<127) {
      tempValue.value=(wchar_t)i;
    }
    else {
      tempValue.value=nonEnglish_Chars[i-127];
    }
    tempValue.count=overallCharFrequencyAnalysis[i];
    sortedOverallFreq.push_back(tempValue);
    tempValue.count=firstCharFrequencyAnalysis[i];
    sortedFirstFreq.push_back(tempValue);
    tempValue.count=lastCharFrequencyAnalysis[i];
    sortedLastFreq.push_back(tempValue);
  }
  sortedFirstFreq.sort(sortFreqFunction);
  sortedLastFreq.sort(sortFreqFunction);
  sortedOverallFreq.sort(sortFreqFunction);
 
//  cout << "Overall Character Frequecy Analysis:" << endl;
  outputFile << "Overall Character Frequecy Analysis:" << endl;
  for (it=sortedOverallFreq.begin();it!=sortedOverallFreq.end();++it) {
    if ((*it).value < 127) {
//      wcout << (*it).value << "\t" << (*it).count/(numberOfCharactersOverall * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersOverall * 1.0) *100 << endl;
    }
    else {
//      wcout << "<Non-ASCII>" << "\t" << (*it).count/(numberOfCharactersOverall * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersOverall * 1.0) *100 << endl;
    }
  }
//  cout << endl << "----------------------------------------\n" << "First Character Frequecy Analysis:" << endl;
  outputFile << endl << "----------------------------------------\n" << "First Character Frequecy Analysis:" << endl;
  for (it=sortedFirstFreq.begin();it!=sortedFirstFreq.end();++it) {
    if ((*it).value < 127) {
//      wcout << (*it).value << "\t" << (*it).count/(numberOfCharactersFirst * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersFirst * 1.0) *100 << endl;
    }
    else {
//      wcout << "<Non-ASCII>" << "\t" << (*it).count/(numberOfCharactersFirst * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersFirst * 1.0) *100 << endl;
    }
  }
//  cout << endl << "----------------------------------------\n" << "Last Character Frequecy Analysis:" << endl;
  outputFile << endl << "----------------------------------------\n" << "Last Character Frequecy Analysis:" << endl;
  for (it=sortedLastFreq.begin();it!=sortedLastFreq.end();++it) {
    if ((*it).value < 127) {
//      wcout << (*it).value << "\t" << (*it).count/(numberOfCharactersLast * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersLast * 1.0) *100 << endl;
    }
    else {
//      wcout << "<Non-ASCII>" << "\t" << (*it).count/(numberOfCharactersLast * 1.0) *100 << endl;
      outputFile << (*it).value << "\t" << (*it).count/(numberOfCharactersLast * 1.0) *100 << endl;
    }
  }
  cout << endl << endl << "Letter Frequency Analysis:\n";
  cout << "Due to some weirdness with C++, when it prints out non-ASCII characters to cout they show up as '?'\n";
  cout << "You can see what they really are if you look at the results.txt file\n";
  cout << "\n\noverall char set to use\n";
  outputFile << endl << endl << "overall char set to use\n";
  for (it=sortedOverallFreq.begin();it!=sortedOverallFreq.end();++it) {
    if ((*it).count > 0) {
      outputFile << (*it).value;
      cout << (char)(*it).value;
    }
  }
  outputFile << endl << endl;
  cout << endl << endl;
  outputFile << "First char set to use\n";
  cout << "First char set to use\n";
  for (it=sortedFirstFreq.begin();it!=sortedFirstFreq.end();++it) {
    if ((*it).count > 0) {
      outputFile << (*it).value;
      cout << (char)(*it).value;
    }
  }
  outputFile << endl << endl;
  cout << endl << endl;
 
  outputFile << "Last char set to use\n";
  cout << "Last char set to use\n";
  for (it=sortedLastFreq.begin();it!=sortedLastFreq.end();++it) {
    if ((*it).count > 0) {
      outputFile << (*it).value;
      cout << (char)(*it).value;
    }
  }
  outputFile << endl << endl;
  cout << endl << endl;


    
  return 0;
} 


bool getNextRecord(wifstream *inputFile, wstring *plainValue) {
  size_t loc;

  if ((*inputFile).eof()) {
    return false;
  }
  getline((*inputFile), *plainValue);
  if ((*inputFile).eof()) {
    return false;
  }
  loc = (*plainValue).find(L"\r");
  if (loc!=wstring::npos) {
    (*plainValue).resize(loc);
//    printf("carrige return found, might want to doublecheck results\n");
  } 
  return true;
}



///////////////////////////////////////////////////
//Attempts to evaluate how complex a password guess is
//0=empty
//+1 for lowercase
//+1 for uppercase
//+1 for digits
//+1 for special characters
//+1 for non-english characters
//+1 for a length of 8 or more
int findBasicComplexity(wstring plainValue) {
  int size;
  int complexityNumber = 0;
  bool isLower=false;
  bool isUpper=false;
  bool isDigit=false;
  bool isSpecial=false;
  bool isNonEnglish=false;
  size=plainValue.size();
  if (size > 7) {
    complexityNumber++;
  }

  for (int i=0;i<size; i++) {
    if ((isLowerChar(plainValue[i]))&&(!isLower)) {
      isLower=true;
      complexityNumber++;
    }
    else if ((isUpperChar(plainValue[i]))&&(!isUpper)) {
      isUpper=true;
      complexityNumber++;
    }
    else if ((isDigitChar(plainValue[i]))&&(!isDigit)) {
      isDigit=true;
      complexityNumber++;
    }
    else if ((isSpecialChar(plainValue[i]))&&(!isSpecial)) {
      isSpecial=true;
      complexityNumber++;
    }
    else if (isNotEnglish(plainValue[i])) {
      isNonEnglish=true;
      complexityNumber++;
    }
  }
  return complexityNumber;
}

bool isComplexPassword(wstring plainValue) {
  bool hasLower=false;
  bool hasUpper=false;
  bool hasSpecial=false;
  bool hasDigit=false;

  if (plainValue.size()<7) {  //basic length check
    return false;
  }
  for (int i=0;i<plainValue.size();i++) {
    if (isLowerChar(plainValue[i])) {
      hasLower=true;
    }
    if (isUpperChar(plainValue[i])) {
      hasUpper=true;
    }
    if (isDigitChar(plainValue[i])) {
      hasDigit=true;
    }
    if (isSpecialChar(plainValue[i])) {
      hasSpecial=true;
    }
  }
  if (hasLower&&hasUpper&&hasDigit&&hasSpecial) {
    return true;
  }
  return false;
}



bool sortFreqFunction(freqType first, freqType second) {
   if (first.count < second.count) {
     return false;
   }
   return true;
}

bool isLowerChar(wchar_t inputChar) {
  if ((inputChar >=96)&&(inputChar <=122)) {
    return true;
  }
  return false;
}

bool isUpperChar(wchar_t inputChar) {
  if ((inputChar >=65)&&(inputChar <=90)) {
    return true;
  }
  return false;
}

bool isDigitChar(wchar_t inputChar) {
  if ((inputChar >=48)&&(inputChar <=57)) {
    return true;
  }
  return false;
}

bool isSpecialChar(wchar_t inputChar) {
  if (((inputChar >=32)&&(inputChar <=47))||((inputChar >=91)&&(inputChar <=96))||((inputChar >=123)&&(inputChar<=126))) {
    return true;
  }
  return false;
}

bool isNotEnglish(wchar_t inputChar) {
  if (inputChar > 127) {
    return true;
  }
  return false;
}


bool measureOverallCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber) {
  int curPos;
  size_t loc;

  for (curPos=0;curPos<inputString.size();curPos++) {
    if (inputString[curPos]<127) { //uses the ascii values
      storageValues[inputString[curPos]]++;
    }
    else {
      loc = (*nonEnglishChars).find(inputString[curPos]);
      if (loc == wstring::npos) { //it's a new non english character
        (*nonEnglishChars).append(1,inputString[curPos]);
        storageValues[126+(*nonEnglishChars).size()]=1;  //it's 126 here since the first value will be at size=1
      }
      else {
        storageValues[127+loc]++;  //it's 127 here since the string loc starts at 0
      }
    }
    (*totalNumber)++;
  }
  return true;
}

bool measureFirstCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber) {
  int curPos;
  size_t loc;

  curPos=0;
  if (inputString[curPos]<127) { //uses the ascii values
    storageValues[inputString[curPos]]++;
  }
  else {
    loc = (*nonEnglishChars).find(inputString[curPos]);
    if (loc == wstring::npos) { //it's a new non english character
      (*nonEnglishChars).append(1,inputString[curPos]);
      storageValues[126+(*nonEnglishChars).size()]=1;  //it's 126 here since the first value will be at size=1
    }
    else {
      storageValues[127+loc]++;  //it's 127 here since the string loc starts at 0
    }
  }
  (*totalNumber)++;
  return true;
}


bool measureLastCharFreq(wstring inputString, long * storageValues, wstring * nonEnglishChars, long *totalNumber) {
  int curPos;
  size_t loc;

  curPos=inputString.size()-1;
  if (inputString[curPos]<127) { //uses the ascii values
    storageValues[inputString[curPos]]++;
  }
  else {
    loc = (*nonEnglishChars).find(inputString[curPos]);
    if (loc == wstring::npos) { //it's a new non english character
      (*nonEnglishChars).append(1,inputString[curPos]);
      storageValues[126+(*nonEnglishChars).size()]=1;  //it's 126 here since the first value will be at size=1
    }
    else {
      storageValues[127+loc]++;  //it's 127 here since the string loc starts at 0
    }
  }
  (*totalNumber)++;
  return true;
}

int hasUpperCase(wstring inputString) {
  
  for (int i=0;i<inputString.size();i++) {
    if (isUpperChar(inputString[i])) {
      return 1;
    }
  }
  return 0;
}


int hasSpecial(wstring inputString) {

  for (int i=0;i<inputString.size();i++) {
    if (isSpecialChar(inputString[i])) {
      return 1;
    }
  }
  return 0;
}

int hasDigit(wstring inputString) {

  for (int i=0;i<inputString.size();i++) {
    if (isDigitChar(inputString[i])) {
      return 1;
    }
  }
  return 0;
}


int onlyLowerCase(wstring inputString) {

  for (int i=0;i<inputString.size();i++) {
    if (!isLowerChar(inputString[i])) {
      return 0;
    }
  }
  return 1;
}

