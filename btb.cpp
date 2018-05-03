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
    unsigned int currentPC; //address of instruction
    unsigned int targetPC;  //address of predicted next instruction
    int prediction;         //prediction state (0-3)
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
    char state_type;
    int collisions;

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
        state_type = 'S';
        collisions = 0;
    }

    void pushIntoBTB(string current, string next)
    {
        checkIfBranch(current, next); //PC going to BTB

        accuracy = 100 * ((double)rights) / hits;                //calculate running accuracy
        hit_percentage = 100 * ((double)hits) / (hits + misses); //calculate running hit %
    }

    friend ostream &operator<<(ostream &os, BTB &theBTB)
    {
        for (int i = 0; i < 1024; i++)
        {
            if (theBTB.predictions[i].index != -1)
            {
                os << theBTB.predictions[i] << endl;
                theBTB.nEntrys++;
            }
        }

        os << endl;
        os << "Hits: " << dec << theBTB.hits << endl;
        os << "Misses: " << dec << theBTB.misses << endl;
        os << "Correct: " << dec << theBTB.rights << endl;
        os << "Wrong: " << dec << theBTB.wrongs << endl;
        os << "Taken: " << dec << theBTB.taken << endl;
        os << "Total: " << dec << theBTB.total << endl;
        os << "Entrys: " << dec << theBTB.nEntrys << endl;
        os << "Accuracy: " << dec << theBTB.accuracy << endl;
        os << "Hit \%: " << dec << theBTB.hit_percentage << endl;
        os << "Collisions: " << dec << theBTB.collisions << endl;
    }

    void checkIfBranch(string current, string next);

    void perform_state_example(bool didBranch, int index, int s1, int s2);
};

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

    bool didBranch;       //so we don't have to keep doing tis
    if (iTwo != iOne + 4) //tell if a branch happened, if the next instruction isn't in order it's a branch
    {
        didBranch = true;
        this->taken++;
    }
    else
        didBranch = false;

    if (this->predictions[index].currentPC != iOne && didBranch && this->predictions[index].busy == true)
    {
        this->collisions++;
    }

    if (this->predictions[index].busy == true && this->predictions[index].currentPC == iOne)
    {
        this->hits++;

        if (this->predictions[index].prediction < 2) //state?
        {
            //SAYS TO BRANCH
            if (didBranch)
            {
                if (this->predictions[index].targetPC == iTwo)
                {
                    this->rights++;
                }
                else
                {
                    this->wrongs++;
                    this->predictions[index].currentPC = iOne; //replace the old one
                    this->predictions[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions[index].prediction = 1;
                    else
                        this->predictions[index].prediction = 0;
                    this->predictions[index].index = index;
                    this->predictions[index].busy = true;
                    return;
                }
            }
            else
            {
                this->wrongs++;
            }
        }
        else
        {
            //SAYS DON'T BRANCH
            if (didBranch)
            {
                if (this->predictions[index].targetPC == iTwo)
                {
                    //correct target but said to not take
                    this->wrongs++;
                }

                else
                {
                    //incorrect target
                    //update
                    this->rights++;
                    this->predictions[index].currentPC = iOne; //replace the old one
                    this->predictions[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions[index].prediction = 1;
                    else
                        this->predictions[index].prediction = 0;
                    this->predictions[index].index = index;
                    this->predictions[index].busy = true;
                    return;
                }
            }
            else
            {
                this->rights++;
            }
        }

        ///update state (expecting no replaced entries)
        switch (state_type)
        {
        case 'S':
            perform_state_example(didBranch, index, 0, 1);
            break;
        case 'B':
            perform_state_example(didBranch, index, 0, 0);
            break;
        case 'D':
            perform_state_example(didBranch, index, 1, 0);
            break;
        default:
            perform_state_example(didBranch, index, 0, 1);
            break;
        }
    }
    else
    {
        if (didBranch)
        {
            this->misses++;
            //update
            //add new entry to BTB
            //this->nEntrys++;
            if (this->state_type == 'B')
                this->predictions[index].prediction = 1;
            else
                this->predictions[index].prediction = 0; //first prediction
            this->predictions[index].busy = true;        //index in use
            this->predictions[index].currentPC = iOne;   //current address
            this->predictions[index].targetPC = iTwo;    //next address;
            this->predictions[index].index = index;
            return;
        }
    }
}

void BTB::perform_state_example(bool didBranch, int index, int s1, int s2)
{
    switch (this->predictions[index].prediction)
    {
    case 0:
        didBranch ? this->predictions[index].prediction = s1 : this->predictions[index].prediction = 1;
        break;
    case 1:
        didBranch ? this->predictions[index].prediction = 0 : this->predictions[index].prediction = 2;
        break;
    case 2:
        didBranch ? this->predictions[index].prediction = s2 : this->predictions[index].prediction = 3;
        break;
    case 3:
        didBranch ? this->predictions[index].prediction = 2 : this->predictions[index].prediction = 3;
        break;
    }
}

class BTB2 //simulated BTB
{

  public:
    unsigned int nEntrys;                            //how many entrys are in BTB, start removing when nEntrys = 1024
    Prediction predictions1[512], predictions2[512]; //array of predictions
    unsigned int hits;                               //number of htis
    unsigned int misses;                             //number of misses
    unsigned int rights;                             //number of correct predictions;
    unsigned int wrongs;                             //number of incorrect predictions;
    unsigned int taken;                              //total number of attempted branches
    unsigned int total;                              //total number of instructions
    double accuracy;                                 //righst/hits
    double hit_percentage;                           //hits / (hits + misses)
    char state_type;
    int collisions;

    unsigned int calculateIndex(unsigned int address); //calculates the expected index (hash-like)

    BTB2() //default constructor
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
        state_type = 'S';
        collisions = 0;
    }

    void pushIntoBTB(string current, string next)
    {
        checkIfBranch(current, next); //PC going to BTB

        accuracy = 100 * ((double)rights) / hits;                //calculate running accuracy
        hit_percentage = 100 * ((double)hits) / (hits + misses); //calculate running hit %
    }

    friend ostream &operator<<(ostream &os, BTB2 &theBTB)
    {
        os << "FIRST BTB" << endl << endl;
        for (int i = 0; i < 512; i++)
        {
            if (theBTB.predictions1[i].index != -1)
            {
                os << theBTB.predictions1[i] << endl;
                theBTB.nEntrys++;
            }
        }
        os << endl << "SECOND BTB" << endl;
        for (int i = 0; i < 512; i++)
        {
            if (theBTB.predictions2[i].index != -1)
            {
                os << theBTB.predictions2[i] << endl;
                theBTB.nEntrys++;
            }
        }

        os << endl;
        os << "Hits: " << dec << theBTB.hits << endl;
        os << "Misses: " << dec << theBTB.misses << endl;
        os << "Correct: " << dec << theBTB.rights << endl;
        os << "Wrong: " << dec << theBTB.wrongs << endl;
        os << "Taken: " << dec << theBTB.taken << endl;
        os << "Total: " << dec << theBTB.total << endl;
        os << "Entrys: " << dec << theBTB.nEntrys << endl;
        os << "Accuracy: " << dec << theBTB.accuracy << endl;
        os << "Hit \%: " << dec << theBTB.hit_percentage << endl;
        os << "Collisions: " << theBTB.collisions << endl;
    }

    void checkIfBranch(string current, string next);

    int perform_state_example(bool didBranch, int s1, int s2, int prediction);
};

unsigned int BTB2::calculateIndex(unsigned int address)
{
    unsigned int temp = address;
    temp = temp & 0x7FF; //shave for 512 address
    temp = temp >> 2;    //shave off the two zeros

    return temp;
}

void BTB2::checkIfBranch(string current, string next) //check if next instruction was a branch or jump
{
    int iOne, iTwo;
    iOne = convertToHex(current);
    iTwo = convertToHex(next);

    if (iOne == iTwo)
    {
        //repeat EOF problem
        return;
    }

    int index = this->calculateIndex(iOne); //current PC index in BTB2
    this->total++;                          //increment instruction count;

    bool didBranch;       //so we don't have to keep doing tis
    if (iTwo != iOne + 4) //tell if a branch happened, if the next instruction isn't in order it's a branch
    {
        didBranch = true;
        this->taken++;
    }
    else
        didBranch = false;

    Prediction *predPtr; //so we can swap between 512-BTB's

    predPtr = this->predictions1;

    if ((this->predictions1[index].currentPC != iOne && didBranch &&
         this->predictions1[index].busy == true) &&
        (this->predictions2[index].currentPC != iOne && didBranch &&
         this->predictions2[index].busy == true))
    {
        this->collisions++;
    }

    //check first btb
    if (this->predictions1[index].busy == true && this->predictions1[index].currentPC == iOne)
    {
        this->hits++;

        if (this->predictions1[index].prediction < 2) //state?
        {
            //SAYS TO BRANCH
            if (didBranch)
            {
                if (this->predictions1[index].targetPC == iTwo)
                {
                    this->rights++;
                }
                else
                {
                    this->wrongs++;
                    this->predictions1[index].currentPC = iOne; //replace the old one
                    this->predictions1[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions1[index].prediction = 1;
                    else
                        this->predictions1[index].prediction = 0;
                    this->predictions1[index].index = index;
                    this->predictions1[index].busy = true;
                    return;
                }
            }
            else
            {
                this->wrongs++;
            }
        }
        else
        {
            //SAYS DON'T BRANCH
            if (didBranch)
            {
                if (this->predictions1[index].targetPC == iTwo)
                {
                    //correct target but said to not take
                    this->wrongs++;
                }

                else
                {
                    //incorrect target
                    //update
                    this->rights++;
                    this->predictions1[index].currentPC = iOne; //replace the old one
                    this->predictions1[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions1[index].prediction = 1;
                    else
                        this->predictions1[index].prediction = 0;
                    this->predictions1[index].index = index;
                    this->predictions1[index].busy = true;
                    return;
                }
            }
            else
            {
                this->rights++;
            }
        }
        int newPred = this->predictions1[index].prediction;
        switch (state_type)
        {
        case 'S':
            this->predictions1[index].prediction = perform_state_example(didBranch, 0, 1, newPred);
            break;
        case 'B':
            this->predictions1[index].prediction = perform_state_example(didBranch, 0, 0, newPred);
            break;
        case 'D':
            this->predictions1[index].prediction = perform_state_example(didBranch, 1, 0, newPred);
            break;
        default:
            this->predictions1[index].prediction = perform_state_example(didBranch, 0, 1, newPred);
            break;
        }
    }

    //check second btb
    else if (this->predictions2[index].busy == true && this->predictions2[index].currentPC == iOne)
    {
        this->hits++;
        predPtr = this->predictions2;
        if (this->predictions2[index].prediction < 2) //state?
        {
            //SAYS TO BRANCH
            if (didBranch)
            {
                if (this->predictions2[index].targetPC == iTwo)
                {
                    this->rights++;
                }
                else
                {
                    this->wrongs++;
                    this->predictions2[index].currentPC = iOne; //replace the old one
                    this->predictions2[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions2[index].prediction = 1;
                    else
                        this->predictions2[index].prediction = 0;
                    this->predictions2[index].index = index;
                    this->predictions2[index].busy = true;
                    return;
                }
            }
            else
            {
                this->wrongs++;
            }
        }
        else
        {
            //SAYS DON'T BRANCH
            if (didBranch)
            {
                if (this->predictions2[index].targetPC == iTwo)
                {
                    //correct target but said to not take
                    this->wrongs++;
                }

                else
                {
                    //incorrect target
                    //update
                    this->rights++;
                    this->predictions2[index].currentPC = iOne; //replace the old one
                    this->predictions2[index].targetPC = iTwo;
                    if (this->state_type == 'B')
                        this->predictions2[index].prediction = 1;
                    else
                        this->predictions2[index].prediction = 0;
                    this->predictions2[index].index = index;
                    this->predictions2[index].busy = true;
                    return;
                }
            }
            else
            {
                this->rights++;
            }
        }

        ///update state (expecting no replaced entries)
        int newPred = this->predictions2[index].prediction;
        switch (state_type)
        {
        case 'S':
            this->predictions2[index].prediction = perform_state_example(didBranch, 0, 1, newPred);
            break;
        case 'B':
            this->predictions2[index].prediction = perform_state_example(didBranch, 0, 0, newPred);
            break;
        case 'D':
            this->predictions2[index].prediction = perform_state_example(didBranch, 1, 0, newPred);
            break;
        default:
            this->predictions2[index].prediction = perform_state_example(didBranch, 0, 1, newPred);
            break;
        }
    }

    //not in either, check for open spot
    else if (this->predictions1[index].busy && didBranch)
    {
        this->misses++;
        //update
        //add new entry to BTB
        //this->nEntrys++;
        if (this->state_type == 'B')
            this->predictions2[index].prediction = 1;
        else
            this->predictions2[index].prediction = 0; //first prediction
        this->predictions2[index].busy = true;        //index in use
        this->predictions2[index].currentPC = iOne;   //current address
        this->predictions2[index].targetPC = iTwo;    //next address;
        this->predictions2[index].index = index;
        //this->collisions++;
        return;
    }
    //place in first btb if branched
    else
    {
        if (didBranch)
        {
            this->misses++;
            //update
            //add new entry to BTB
            //this->nEntrys++;
            if (this->state_type == 'B')
                this->predictions1[index].prediction = 1;
            else
                this->predictions1[index].prediction = 0; //first prediction
            this->predictions1[index].busy = true;        //index in use
            this->predictions1[index].currentPC = iOne;   //current address
            this->predictions1[index].targetPC = iTwo;    //next address;
            this->predictions1[index].index = index;
            return;
        }
    }
}

int BTB2::perform_state_example(bool didBranch, int s1, int s2, int prediction)
{
    int nprediction;

    switch (prediction)
    {
    case 0:
        didBranch ? nprediction = s1 : nprediction = 1;
        break;
    case 1:
        didBranch ? nprediction = 0 : nprediction = 2;
        break;
    case 2:
        didBranch ? nprediction = s2 : nprediction = 3;
        break;
    case 3:
        didBranch ? nprediction = 2 : nprediction = 3;
        break;
    }

    return nprediction;
}

int main()
{
    //94.6105
    BTB dBTestS, dDTestS, lBTestS, lDTestS;
    BTB2 dBTest, dDTest, lBTest, lDTest;
    dBTest.state_type = 'B'; //D: Accuracy: 94.1888 Hit %: 95.3063
    dDTest.state_type = 'D';
    lBTest.state_type = 'B';
    lDTest.state_type = 'D';
    dBTestS.state_type = 'B'; //D: Accuracy: 94.1888 Hit %: 95.3063
    dDTestS.state_type = 'D';
    lBTestS.state_type = 'B';
    lDTestS.state_type = 'D';

    //input files
    ifstream doduc, li_int;
    //opening
    doduc.open("Doduc_benchmark.txt");
    li_int.open("022.li_int_text2.txt");

    //output files
    ofstream doduc_B_1024_out, doduc_B_512_out, doduc_D_1024_out, doduc_D_512_out;
    ofstream li_B_1024_out, li_B_512_out, li_D_1024_out, li_D_512_out;
    //opening
    doduc_B_1024_out.open("doduc_B_1024.txt");
    doduc_B_512_out.open("doduc_B_512.txt");
    doduc_D_1024_out.open("doduc_D_1024.txt");
    doduc_D_512_out.open("doduc_D_512.txt");
    li_B_1024_out.open("li_B_1024.txt");
    li_B_512_out.open("li_B_512.txt");
    li_D_1024_out.open("li_D_1024.txt");
    li_D_512_out.open("li_D_512.txt");

    string s1, s2;

    while (li_int)
    {
        getline(li_int, s1);                     //read for current instruction
        streampos nextPosition = li_int.tellg(); //store the next position
        getline(li_int, s2);                     //read further for next instruction
        li_int.seekg(nextPosition);              //return to previos instruction
        lBTest.pushIntoBTB(s1, s2);
        lDTest.pushIntoBTB(s1, s2);
        lBTestS.pushIntoBTB(s1, s2);
        lDTestS.pushIntoBTB(s1, s2);
    }

    while (doduc)
    {
        getline(doduc, s1);                     //read for current instruction
        streampos nextPosition = doduc.tellg(); //store the next position
        getline(doduc, s2);                     //read further for next instruction
        doduc.seekg(nextPosition);              //return to previos instruction
        dBTest.pushIntoBTB(s1, s2);
        dDTest.pushIntoBTB(s1, s2);
        dBTestS.pushIntoBTB(s1, s2);
        dDTestS.pushIntoBTB(s1, s2);
    }

    //OUTPUT TO FILES
    doduc_B_1024_out << dBTestS << endl;
    doduc_D_1024_out << dDTestS << endl;
    li_B_1024_out << lBTestS << endl;
    li_D_1024_out << lDTestS << endl;
    doduc_B_512_out << dBTest << endl;
    doduc_D_512_out << dDTest << endl;
    li_B_512_out << lBTest << endl;
    li_D_512_out << lDTest << endl;

    return 0;
}