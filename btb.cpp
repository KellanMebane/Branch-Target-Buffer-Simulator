#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;
int convertToHex(string input);

class Prediction //instructions in BTB
{

  public:
    unsigned int currentPC;  //address of instruction
    unsigned int targetPC;   //address of predicted next instruction
    int prediction; //prediction state (0-3)
    bool busy;
    int index;

    Prediction() //default constructor
    {
        currentPC = 0;
        targetPC = 0;
        prediction = 0;
        index = -1; //not in a BTB
        busy = false;
    }

    Prediction(unsigned int cur, unsigned int tar, int pred, bool b, int index) //constructor
    {
        currentPC = cur;
        targetPC = tar;
        prediction = pred;
        busy = b;
        index = index;
    }

    friend ostream &operator<<(ostream &os, const Prediction &pred)
    {
        os << left << setw(14) << "|Index|"
           << setw(14) << "|PC|"
           << setw(14) << "|Target|"
           << setw(14) << "|Prediction|" << endl;

        if (pred.index != -1)
            os << left << dec << setw(14) << pred.index;
        else
            os << left << setw(14) << "unmarked";

        os << left << hex
           << setw(14) << pred.currentPC
           << setw(14) << pred.targetPC
           << setw(14) << pred.prediction << endl;

        os << left << setw(14) << "--------------"
           << setw(14) << "--------------"
           << setw(14) << "--------------"
           << setw(14) << "------------";
    }
};

class BTB //simulated BTB
{

  public:
    unsigned int nEntrys;         //how many entrys are in BTB, start removing when nEntrys = 1024
    Prediction predictions[1024]; //array of predictions
    unsigned int hits;            //number of htis
    unsigned int misses;          //number of misses
    unsigned int rights;          //number of correct predictions;
    unsigned int wrongs;          //number of incorrect predictions;
    unsigned int taken;           //total number of attempted branches
    unsigned int total;           //total number of instructions
    double accuracy;              //righst/hits
    double hit_percentage;        //hits / (hits + misses)

    unsigned int calculateIndex(unsigned int address); //calculates the expected index (hash-like)

    BTB() //default constructor
    {
        nEntrys = 0;
        hits = 0;
        misses = 0;
        taken = 0;
        rights = 0;
        wrongs = 0;
        total = 0;
        accuracy = 0;
        hit_percentage = 0;
    }

    void pushIntoBTB(string current, string next)
    {
        checkIfBranch(current, next); //PC going to BTB

        accuracy = 100 * ((double)rights) / hits;                //calculate running accuracy
        hit_percentage = 100 * ((double)hits) / (hits + misses); //calculate running hit %
    }

    friend ostream &operator<<(ostream &os, const BTB &theBTB)
    {
        os << "Hits: " << theBTB.hits << endl;
        os << "Misses: " << theBTB.misses << endl;
        os << "Correct: " << theBTB.rights << endl;
        os << "Wrong: " << theBTB.wrongs << endl;
        os << "Taken: " << theBTB.taken << endl;
        os << "Total: " << theBTB.total << endl;
        os << "Entrys: " << theBTB.nEntrys << endl;
        os << "Accuracy: " << theBTB.accuracy << endl;
        os << "Hit \%: " << theBTB.hit_percentage << endl
           << endl;

        for (int i = 0; i < 1024; i++)
        {
            if (theBTB.predictions[i].index != -1)
                os << theBTB.predictions[i] << endl;
        }
        os << endl;
    }

    void checkIfBranch(string current, string next);
};

int main()
{
    BTB branchTest;
    ifstream input;
    input.open("trace_sample.txt");
    ofstream output;
    output.open("test.txt");
    string s1, s2;

    while (input)
    {
        getline(input, s1);                     //read for current instruction
        streampos nextPosition = input.tellg(); //store the next position
        getline(input, s2);                     //read further for next instruction
        input.seekg(nextPosition);              //return to previos instruction
        branchTest.pushIntoBTB(s1, s2);
    };

        output << branchTest << endl;

    return 0;
}

unsigned int BTB::calculateIndex(unsigned int address)
{
    unsigned int temp = address;
    temp = temp & 0xFFF; //get lower 3 bits
    temp = temp >> 2;    //shave off the two zeros

    return temp;
}

int convertToHex(string input) //take the hex value of what a string represents
{
    stringstream str;
    str << input; //load into stream
    int value;
    str >> hex >> value; //convert to hex

    return value; //return the integer
}

void BTB::checkIfBranch(string current, string next) //check if next instruction was a branch or jump
{
    int iOne, iTwo;
    iOne = convertToHex(current);
    iTwo = convertToHex(next);
    
    if (iOne == iTwo)
    {
        //repeat EOF problem
        return;
    }

    int index = this->calculateIndex(iOne); //current PC index in BTB
    this->total++;                          //increment instruction count;

    bool didBranch; //so we don't have to keep doing tis
    if (iTwo == iOne + 4) //tell if a branch happened, if the next instruction isn't in order it's a branch
        didBranch = true;
    else
        didBranch = false;

    if (this->predictions[index].busy == true) //busy true means index is in use (we have a prediction)
    {

    }
    else
    {
        //no prediction for this PC yet
        if (didBranch)
        {
            //miss
            //update
            return;
        }

        
    }

}