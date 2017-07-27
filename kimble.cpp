/**
 * @file   kimble.cpp
 * @Author Ashish Sultania (ashishsultania2k7@gmail.com)
 * @date   26 July, 2017
 * @brief  Code for Kimble game.
 * @time   Approx 8hours including debuggings
 * @ref    http://www.cplusplus.com/reference/algorithm/
 * In the actual game, it is possible for the player to select which peg to move. For simplicity, the Simulator will use a simple strategy of always moving the peg that is closest to the player's finish line if possible.
 */

#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

/* Forward Declaration */
class Board;
Board** player;
void check_otherplayerpeg(Board** player, int pid, int gid, int peg_id, int num_game, int num_player);

#define NUM_PEG 4          /* Number of Pegs */
#define NUM_BOARDBOX 28    /* Number of boxes in board */
#define NUM_FLINE 4        /* Number of finishing boxes  */

/**
 * @fn      pop_o_matic()
 * @brief   Simulate the Pop-O-Matic.
 *
 * @param [in] None
 *
 * @retval Random value of range 1-6.
 *
 */
int pop_o_matic()
{
    return ( 1 + rand()%6);
}

/**
 * @fn      count()
 * @brief   Function to check number of occurance of any number in an array.
 *
 * @param [in] number         : Number of check
 *        [in] array          : Array
 *        [in] length         : Length of array
 *        [out] occurance     : Number of occurance of the number
 *
 * @retval None.
 *
 */

void count(int number, int array[], int length, int &occurence)
{
    if (*array == number)
    {
        ++occurence;
    }

    if (1 == length)
    {
        return;
    }
    else    
    {
        count(number, array+1, length-1, occurence);
    }
}



/**
 * @class   Board
 * @brief   Game board configuaration class
 *
 */
class Board
{
    public:
        /* Constructor */
        Board()
        {
            auto init1 = std::initializer_list<int>({-1,-1,-1,-1});
            auto init2 = std::initializer_list<int>({0,0,0,0});

            std::copy(init1.begin(), init1.end(), peg_position);
            std::copy(init2.begin(), init2.end(), peg_finished);
            std::copy(init2.begin(), init2.end(), peg_zone);
        };
        ~Board(){};
        
        /**
         * @fn      update_peg_position()
         * @brief   Update the peg position.
         *
         * @param [in] peg_id       : Peg ID   
         *        [in] value        : Movement side for Peg
         *        [in] player       : Object of Board
         *        [in] pid          : Current player identifier
         *        [in] gid          : Current Set identifier
         *        [in] num_game     : Number of set 
         *        [in] num_player   : Number of Player
         *
         * @retval None
         *
         */
        void update_peg_position(int peg_id, int value, Board** player, int pid, int gid, int num_game, int num_player)
        {
            
            /* Case when peg is in home */
            if (-1 == peg_position[peg_id] && (6 == value) )
            {
                peg_position[peg_id] = 0;
            }
            
            /* peg_id >3 means all the pegs has been checked and no one can move */
            else if (-1 == peg_position[peg_id] && (6 != value) || peg_id > 3)
            {
                //Do Nothing
                
            }
            
            /* Case when peg is finishing*/
            else if ( (num_moves+num_finishinglines) == peg_position[peg_id] + value )
            {
                peg_position[peg_id] += value;
                peg_finished[peg_id] = 1;
            }
            
            /* Case when the current peg can't move */
            else if ( (num_moves + num_finishinglines - peg_position[peg_id]) < value )
            {
                int num_zero_zone = -2;
                int num_minus1_pp = -3;
                
                /* Count number of pegs which finished their journey */
                count(0, player[pid][gid].peg_zone, sizeof(player[pid][gid].peg_zone)/sizeof(player[pid][gid].peg_zone[0]), num_zero_zone);
                
                /* Count number of pegs which are inactive */
                count(-1, player[pid][gid].peg_position, sizeof(player[pid][gid].peg_position)/sizeof(player[pid][gid].peg_position[0]), num_minus1_pp);

                
                if (6 != value && num_minus1_pp == num_zero_zone)
                {
                    cout<<"Can't move any peg in this chance, better luck next time"<<num_minus1_pp<<" "<<num_zero_zone<<"\n";
                }
                
             
                /* Move another peg */
                else
                {
                    int prev_peg_id = peg_id;
                    peg_id = get_next_peg_tomove(peg_id+1);
                    if (peg_id == prev_peg_id || 3 < peg_id)
                    {
                        return;
                    }

                    update_peg_position(peg_id, value,player,pid,gid,num_game,num_player);
                    get_gamestatus();
                    check_otherplayerpeg(player, pid, gid, peg_id, num_game, num_player);        
                }
            }
            else
            {
                peg_position[peg_id] += value;
                
                /* Peg finished its journey */
                if(num_moves + num_finishinglines == peg_position[peg_id])
                {
                    peg_finished[peg_id] = 1;
                }
            }
            
            /* Change zone of Peg to 'save mode' */
            if(peg_position[peg_id] >= num_moves)
            {
                peg_zone[peg_id] = 1;
            }
            return;
        }
        
        /**
         * @fn      get_next_peg_tomove()
         * @brief   Function to get peg identifier to move for the current chance 
         *          //return  peg position id and if one of the peg finished then don't consider it
         * @param [in] index        : Current peg identifier   
         *
         * @retval [Integer]  x     : Maximum index for Peg identifier.
         *
         */
        int get_next_peg_tomove(int index)
        {
            int temp[NUM_PEG];            /* Temporary variable for storing the peg_position */
            int x;                        /* Return index value */
            int matchedindex  = -1;
            int nummatch      = 0;
            
            /* Copy peg_position to Temporary array */
            std::partial_sort_copy(peg_position, peg_position + NUM_PEG, temp, temp + NUM_PEG);
            std::reverse(temp, temp + NUM_PEG);
            
            /* Get the number of matches and last matched index */
            for (int i=0; i< NUM_PEG; i++) 
            {
                if (temp[index] == peg_position[i]) 
                {
                    nummatch++;
                    matchedindex = i;
                }
            }
            
            /* Case scenario like when peg_position = {4,4,-1,-1}*/
            if ( 1< nummatch  && -1 != peg_position[matchedindex])
            {
                x = std::distance(peg_position, &peg_position[matchedindex]);
            }
            
            else
            {
                x = std::distance(peg_position, std::find(peg_position, peg_position + 4, temp[index]));
            }
            
            if (1 == peg_finished[x])
            {
                x = get_next_peg_tomove(x+1);
            }            
            return x;
        }
        
        void get_gamestatus()
        {
            cout<<"\nNew position: ";
            copy(peg_position, peg_position + sizeof(peg_position) / sizeof(peg_position[0]), ostream_iterator<int>(cout, ","));
            
            cout<<"  Peg Finished: ";
            copy(peg_finished, peg_finished + sizeof(peg_finished) / sizeof(peg_finished[0]), ostream_iterator<int>(cout, ","));
            
            cout<<"  Peg Zone: ";
            copy(peg_zone, peg_zone + sizeof(peg_zone) / sizeof(peg_zone[0]), ostream_iterator<int>(cout, ","));
            cout<<"\n";
            
        }
        
        int peg_finished[NUM_PEG];  /* Indicator whether peg is finished (0=not; 1=yes) */
        int peg_position[NUM_PEG];  /* Indicator of peg position -1 = inactive, 0 = home, +ve means on board */
        int peg_zone[NUM_PEG];      /*  Indicator of peg zone (0=In Normal zone, 1= In save zone i.e finishing lines)*/
        const int num_moves           = NUM_BOARDBOX; /*  Number of board boxes*/
        const int num_finishinglines  = NUM_FLINE; /* Number of finishing boxes  */
};

/**
 * @fn      check_otherplayerpeg()
 * @brief   Function to check to cancel other player's peg which are present on the same position as current player.
 *
 * @param [in] player       : Object of Board
 *        [in] pid          : Current player identifier
 *        [in] gid          : Current Set identifier
 *        [in] num_game     : Number of set 
 *        [in] num_player   : Number of Player
 *
 * @retval None.
 *
 */

void check_otherplayerpeg(Board** player, int pid, int gid, int peg_id, int num_game, int num_player)
{
    
    int currentppp  = player[pid][gid].peg_position[peg_id];
    int currentppz  = player[pid][gid].peg_zone[peg_id];
    int currentpno  = 2*gid + pid;  /* Player number*/
    
    /* Case when peg is in its save zone*/
    if(1 == currentppz)
    {
        return;
    }
    
    for(int g = 0; g < num_game; g++)
    {
        for (int p = 0; p <num_player; p++)
        {
            for (int peg=0; peg <NUM_PEG; peg++)
            {
                int playerno  = 2*g + p;
                int diff      = (playerno - currentpno + NUM_PEG)%NUM_PEG;
                
                /* Case when playerno = 2 = currentpno */
                if (0 == diff)
                {
                    diff = 3;
                }

                /* Skip Current player */
                if (p==pid && g==gid)
                {
                    continue;
                }
                else
                {
                    /* Condition for same peg_positions */
                    if( (player[p][g].peg_position[peg] + diff == currentppp) && (-1 != player[p][g].peg_position[peg]) && (0 == player[p][g].peg_zone[peg]))
                    {
                        cout<<"Cancelling Peg:"<<player[p][g].peg_position[peg]<<" of player:"<<p<<" Set:"<<g<<"\n";
                        player[p][g].peg_position[peg] = -1;
                    }
                }
            }
        }
    }
}



int main()
{
    int selector    = 1;  /* User entry done */
    int num_player  = 0;  
    int num_game;         /* Number of sets */
    int gamestatus  = 1;  /* 1= Game is Running; 0 = Game is over*/
    int dice_val;         /* Current dice value from PoP-O-Matic */
    int peg_id;
    int occurence_pos;
    int occurence_fin;
    
    cout<<"************************************************ \n";
    cout<<"Welcome to the Kimble Simulator design by Ashish \n";
    cout<<"************************************************\n \n";
    
    /* Selecting players  and game */
    while(selector)
    {
        cout<<"Please select the number of Players: ";
        cin>>num_player;
        num_game = 1;
        if (4 < num_player)
        {
            cout<<"Error: Number of player can't be more than 4  \n";
            cout<<"Please select in 2 to 4 \n";
            continue;
        }
        if (1 == num_player)
        {
            cout<<"Error: Number of player can't be 1 \n";
            cout<<"Please select in 2 to 4 \n";
            continue;
        }
        if (2 == num_player)
        {
            cout<<"Please select the number of sets associated to each players: ";
            cin>>num_game;
            
            if (2<num_game)
            {
                cout<<"Error: Number of can't be greater than 2 \n";
                cout<<"Retry \n";
                continue;
            }
        }
        
        selector = 0;
    }


    /* Adding players to board */
    Board** player = new Board*[num_player];
    
    for(int i = 0; i<num_player; i++)
    {
        player[i] = new Board[num_game];
    }
    
    /* Start Game */
    while(1 == gamestatus)
    {
        for(int gid = 0; gid < num_game && gamestatus; gid++)
        {
            for (int pid = 0; pid <num_player && gamestatus; pid++)
            {
                peg_id          = -1;
                occurence_pos   = 0;
                occurence_fin   = 0;
                
                cout<<"-----------------------------------------------------------------";
                cout<<"\nTurn of player:"<<pid<<" for set:"<<gid<<" \n";
                cout<<"Press enter to PoP-O-Matic: ";
                cin.get();

                dice_val = pop_o_matic();
                cout<<"You got: "<<dice_val<<"\n";
                
                /* Count number of pegs which are inactive */
                count(-1, player[pid][gid].peg_position, sizeof(player[pid][gid].peg_position)/sizeof(player[pid][gid].peg_position[0]),occurence_pos);
                
                /* Count number of pegs which finished their journey */
                count(0, player[pid][gid].peg_finished, sizeof(player[pid][gid].peg_finished)/sizeof(player[pid][gid].peg_finished[0]),occurence_fin);
                
                /* Case when number of pegs required to move are inactive, and the Current dice value is not 6 */
                if( (occurence_fin == occurence_pos)  && (6 != dice_val) )
                {
                    cout<<"You have to wait for your next turn, best of luck for getting 6 \n";
                }                
                
                /* Case when Normal case OR number of pegs required to move are inactive, and the Current dice value is 6   */
                else if( (occurence_fin != occurence_pos) || ((occurence_fin == occurence_pos)  && (6 == dice_val)) )
                {
                    peg_id = player[pid][gid].get_next_peg_tomove(0);
                    player[pid][gid].update_peg_position(peg_id, dice_val, player, pid, gid, num_game ,num_player);
                    check_otherplayerpeg(player, pid, gid, peg_id, num_game, num_player);
                    
                    player[pid][gid].get_gamestatus();
                    
              
                    while (6 == dice_val)
                    {
                        cout<<"You got one more chance to enter the PoP-O-Matic \n";
                        cin.get();
                        dice_val = pop_o_matic();
                        cout<<"You got: "<<dice_val<<"\n";
                        peg_id = player[pid][gid].get_next_peg_tomove(0);
                        player[pid][gid].update_peg_position(peg_id, dice_val, player, pid, gid, num_game,num_player);
                        check_otherplayerpeg(player, pid, gid, peg_id, num_game, num_player);
    
                        player[pid][gid].get_gamestatus();
                        
                    }
                    
                }
                
                int comp[] = {1,1,1,1};
            
                if (std::equal(std::begin(player[pid][gid].peg_finished), std::end(player[pid][gid].peg_finished), std::begin(comp)))
                {
                    cout<<"\n\n\n************************************************************\n";
                    cout<<"Player: "<<pid<<" wins for set:"<<gid<<"\n";
                    cout<<"************************************************************\n";
                    /* Game Finished */
                    gamestatus = 0;
                }
            }
        }
    }
    
    cout<<"GAME SUMMARY\n \n";
    for(int gid = 0; gid < num_game; gid++)
    {
        for (int pid = 0; pid <num_player; pid++)
        {
            cout<<"\nPlayer:"<<pid<<" Set:"<<gid;
            player[pid][gid].get_gamestatus();
        }
    }            
return 0;
};