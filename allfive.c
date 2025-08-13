#include <stdio.h>
#include <time.h>
#include "poker.h"

/****************************************************************
    This code tests my evaluator by looping over all 2,598,960
    possible five-card poker hands, calculating each hand's
    distinct value, and displaying the frequency count of each
    hand type.  It also prints the amount of time taken to
    perform all the calculations.

    Kevin L. Suffecool (a.k.a "Cactus Kev"), 2001
    kevin@suffe.cool
****************************************************************/

// The expected frequency count for each hand rank.
int expected_freq[] =
{
    0, 40, 624, 3744, 5108, 10200, 54912, 123552, 1098240, 1302540
};


int
main()
{
    int deck[52], hand[5], freq[10];
    int value, n;
    struct timespec start, end;
    unsigned long elapsed_nsec;

    // Initialize the deck.
    init_deck(deck);

    // Zero out the frequency array.
    for (int i = 0; i < 10; i++)
        freq[i] = 0;

    // Capture start time.
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Loop over every possible five-card hand.
    for (int a = 0; a < 48; a++)
    {
        hand[0] = deck[a];
        for (int b = a+1; b < 49; b++)
        {
            hand[1] = deck[b];
            for (int c = b+1; c < 50; c++)
            {
                hand[2] = deck[c];
                for (int d = c+1; d < 51; d++)
                {
                    hand[3] = deck[d];
                    for (int e = d+1; e < 52; e++)
                    {
                        hand[4] = deck[e];

                        value = eval_5hand(hand);
                        n = hand_rank(value);
                        //print_hand(hand, 5);
                        //printf("  %s\n", value_str[n]);
                        freq[n]++;
                    }
                }
            }
        }
    }

    // Capture end time.
    clock_gettime(CLOCK_MONOTONIC, &end);

    for (int i = 1; i <= 9; i++) {
        printf("%15s: %8d", value_str[i], freq[i]);
        if (freq[i] != expected_freq[i])
            printf(" (expected %d)\n", expected_freq[i]);
        else
            printf("\n");
    }

    // Calculate elapsed time (in nanoseconds).
    if (end.tv_sec == start.tv_sec)
        elapsed_nsec = end.tv_nsec - start.tv_nsec;
    else {
        int sec = end.tv_sec - start.tv_sec;
        elapsed_nsec = (end.tv_nsec + 1000000000 * sec) - start.tv_nsec;
    }

    // Print elapsed time in milliseconds.
    printf("\nElapsed time: %.4f (msecs)\n", (double)(0.000001 * elapsed_nsec));

    return 0;
}
