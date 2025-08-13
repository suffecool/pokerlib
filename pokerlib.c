#include <stdio.h>
#include <stdlib.h>
#include "arrays.h"
#include "poker.h"

// Poker hand evaluator
//
// Kevin L. Suffecool (a.k.a. "Cactus Kev")
// kevin@suffe.cool
//

void srand48();
double drand48();

//
//   This routine initializes the deck.  A deck of cards is simply
//   an integer array of length 52 (no jokers).  This array is
//   populated with each card, using the following scheme:
//
//   An integer is made up of four bytes.  The high-order bytes are
//   used to hold the rank bit pattern, whereas the low-order bytes
//   hold the suit/rank/prime value of the card.
//
//   +--------+--------+--------+--------+
//   |xbbbbbbb|bbbbbbxx|cdhsrrrr|xxpppppp|
//   +--------+--------+--------+--------+
//
//   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
//   r = rank of card (deuce=2,trey=3,four=4,five=5,...,ace=14)
//   cdhs = suit of card
//   b = bit turned on depending on rank of card
//
//   As an example, the Five of Hearts would be represented as:
//
//   +--------+--------+--------+--------+
//   |00000000|00100000|00100101|00000111| = 0x00202507
//   +--------+--------+--------+--------+
//
//   and the Queen of Clubs would be represented as:
//
//   +--------+--------+--------+--------+
//   |00010000|00000000|10001100|00011111| = 0x10008C1F
//   +--------+--------+--------+--------+
//
void
init_deck(int *deck)
{
    int n = 0, suit = CLUB;

    for (int i = 0; i < 4; i++, suit >>= 1)
        for (int j = 0; j < 13; j++, n++)
            deck[n] = primes[j] | ((2+j) << 8) | suit | (1 << (16+j));
}


//  This routine will search a deck for a specific card
//  (specified by rank/suit), and return the INDEX giving
//  the position of the found card.  If it is not found,
//  then it returns -1.
//
int
find_card(int rank, int suit, int *deck)
{
    for (int i = 0; i < 52; i++)
    {
        int c = deck[i];
        if ((c & suit) && (RANK(c) == rank))
            return i;
    }
    return -1;
}


//
//  This routine takes a deck and randomly mixes up
//  the order of the cards.
//
void
shuffle_deck(int *deck)
{
    int n, temp[52];

    for (int i = 0; i < 52; i++)
        temp[i] = deck[i];

    for (int i = 0; i < 52; i++)
    {
        do {
            n = (int)(51.9999999 * drand48());
        } while (temp[n] == 0);
        deck[i] = temp[n];
        temp[n] = 0;
    }
}


//
//  This routine prints the given hand as a string; e.g.
//
//      Ac 4d 7c Jh 2s
//
//  It does NOT print a trailing newline.
//
void
print_hand(int *hand, int n)
{
    char suit;
    static char *rank = "??23456789TJQKA?";

    for (int i = 0; i < n; i++, hand++)
    {
        int r = (*hand >> 8) & 0xF;
        if (*hand & CLUB)
            suit = 'c';
        else if (*hand & DIAMOND)
            suit = 'd';
        else if (*hand & HEART)
            suit = 'h';
        else
            suit = 's';

        printf("%c%c ", rank[r], suit);
    }
}


// Returns the hand rank of the given equivalence class value.
// Note: the parameter "val" should be in the range of 1-7462.
int
hand_rank(unsigned short val)
{
    if (val > 6185) return HIGH_CARD;        // 1277 high card
    if (val > 3325) return ONE_PAIR;         // 2860 one pair
    if (val > 2467) return TWO_PAIR;         //  858 two pair
    if (val > 1609) return THREE_OF_A_KIND;  //  858 three-kind
    if (val > 1599) return STRAIGHT;         //   10 straights
    if (val > 322)  return FLUSH;            // 1277 flushes
    if (val > 166)  return FULL_HOUSE;       //  156 full house
    if (val > 10)   return FOUR_OF_A_KIND;   //  156 four-kind
    return STRAIGHT_FLUSH;                   //   10 straight-flushes
}

// Perform a perfect hash lookup (courtesy of Paul Senzee).
static unsigned
find_fast(unsigned u)
{
    unsigned a, b, r;

    u += 0xe91aaa35;
    u ^= u >> 16;
    u += u << 8;
    u ^= u >> 4;
    b  = (u >> 8) & 0x1ff;
    a  = (u + (u << 2)) >> 19;
    r  = a ^ hash_adjust[b];
    return r;
}


// Evaluates the given five-card poker cards.
// Returns a number from 1 to 7462, where 1 is the best hand
// possible (i.e. Royal Flush), and 7462 is the worst hand
// possible (75432 unsuited).
static unsigned short
eval_5cards(int c1, int c2, int c3, int c4, int c5)
{
    int q = (c1 | c2 | c3 | c4 | c5) >> 16;
    short s;

    // This checks for Flushes and Straight Flushes.
    if (c1 & c2 & c3 & c4 & c5 & 0xf000)
        return flushes[q];

    // This checks for Straights and High Card hands.
    if ((s = unique5[q]))
        return s;

    // This performs a perfect-hash lookup for remaining hands.
    q = (c1 & 0xff) * (c2 & 0xff) * (c3 & 0xff) * (c4 & 0xff) * (c5 & 0xff);
    return hash_values[find_fast(q)];
}

// Evaluates the given five-card poker hand array.
unsigned short
eval_5hand(int *hand)
{
    int c1 = *hand++;
    int c2 = *hand++;
    int c3 = *hand++;
    int c4 = *hand++;
    int c5 = *hand;

    return eval_5cards(c1, c2, c3, c4, c5);
}


// This is a non-optimized method of determining the
// best five-card hand possible out of seven cards.
// I am working on a faster algorithm.
//
unsigned short
eval_7hand(int *hand)
{
    int subhand[5];
    unsigned short best = 9999;

    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 5; j++)
            subhand[j] = hand[ perm7[i][j] ];
        unsigned short q = eval_5hand(subhand);
        if (q < best)
            best = q;
    }
    return best;
}
