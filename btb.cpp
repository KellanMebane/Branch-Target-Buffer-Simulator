#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
int convertToHex(string input);

class Prediction //instructions in BTB
{

  public:
    unsigned int currentPC; //address of instruction
    unsigned int targetPC;  //address of predicted next instruction
    int prediction;         //prediction state (0-3)
    bool busy;

    Prediction() //default constructor
    {
        currentPC = 0;
        targetPC = 0;
        prediction = 0;
        busy = false;
    }

    Prediction(unsigned int cur, unsigned int tar, unsigned int pred, bool b) //constructor
    {
        currentPC = cur;
        targetPC = tar;
        prediction = pred;
        busy = b;
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
    }

    void checkIfBranch(string current, string next);
};

int main()
{
    BTB branchTest;
    ifstream input;
    input.open("trace_sample.txt");
    string s1, s2;

    while (input)
    {
        getline(input, s1);                     //read for current instruction
        streampos nextPosition = input.tellg(); //store the next position
        getline(input, s2);                     //read further for next instruction
        input.seekg(nextPosition);              //return to previos instruction
        branchTest.checkIfBranch(s1, s2);
    };

    cout << "Hits: " << branchTest.hits << endl;
    cout << "Misses: " << branchTest.misses << endl;
    cout << "Correct: " << branchTest.rights << endl;
    cout << "Wrong: " << branchTest.wrongs << endl;
    cout << "Taken: " << branchTest.taken << endl;
    cout << "Total: " << branchTest.total << endl;

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
    int index = this->calculateIndex(iOne); //current PC index in BTB
    this->total++;                          //increment instruction count;

    if (this->predictions[index].busy == true) //prediction exist;
    {

        this->hits++; //incrememnt BTB hits

        //case of same mapping
        if (iOne != this->predictions[index].currentPC) //there is a prediction, but it doesn't match the current PC
        {
            this->wrongs++;
            if (iTwo == iOne + 4) //branch didn't happen
            {
                //do nothing
                return;
            }
            this->predictions[index].currentPC = iOne; //replace the old one
            this->predictions[index].targetPC = iTwo;
            this->predictions[index].prediction = 0;
            //stall?
            return;
        }

        if (this->predictions[index].targetPC == iTwo) //prediction was correct
        {
            //ask about this
            this->rights++; //increment correct predictions

            if (this->predictions[index].prediction > 1) //prediction was previously wrong. decrement, but don't take
            {
                this->predictions[index].prediction--; //decrement prediction
                //stall? BECAUSE WRONG
                return;
            }
            else //prediction was previously correct
            {
                this->taken++;                               //branch was taken
                this->predictions[index].prediction--;       //decrement prediction
                if (this->predictions[index].prediction < 0) //lower bound 0
                    this->predictions[index].prediction = 0;
                return;
            }
        }
        else //prediction was incorrect (NO BRANCH)
        {
            //ask about this
            this->wrongs++;                              //increment wrong predictions
            if (this->predictions[index].prediction > 1) //prediction was previously wrong.
            {
                this->predictions[index].prediction++;       //increment prediction state
                if (this->predictions[index].prediction > 3) //upper bound 3
                    this->predictions[index].prediction = 3;
                //stall?
                return;
            }
            else //prediction was previously correct
            {
                this->taken++;                         //branch was taken
                this->predictions[index].prediction++; //increment prediction
                //stall? BECAUSE WRONG
                return;
            }
        }
    }
    else //prediction doesn't exist
    {
        if (iTwo != iOne + 4) //branch happened
        {
            this->taken++; //branch was taken
            this->misses++;
            //add new entry to BTB
            this->predictions[index].prediction = 0;   //first prediction
            this->predictions[index].busy = true;      //index in use
            this->predictions[index].currentPC = iOne; //current address
            this->predictions[index].targetPC = iTwo;  //next address;
            //stall?
            return;
        }
        else //branch didn't happen
        {
            //do nothing
            return;
        }
    }
}