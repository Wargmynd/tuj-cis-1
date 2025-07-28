#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_FILE "data/euler54.txt"

typedef struct {
    int value; // 2-14 (2-10, J=11, Q=12, K=13, A=14)
    char suit; // 'H', 'D', 'C', 'S'
} Card;

typedef struct {
    Card cards[5];
} Hand;

// Convert card character to value
int card_value(char c) {
    if (c >= '2' && c <= '9') return c - '0';
    if (c == 'T') return 10;
    if (c == 'J') return 11;
    if (c == 'Q') return 12;
    if (c == 'K') return 13;
    if (c == 'A') return 14;
    return 0;
}

// Compare function for qsort (descending)
int cmp(const void *a, const void *b) {
    return ((Card*)b)->value - ((Card*)a)->value;
}

// Parse 5 cards from tokens into a Hand
void parse_hand(char **tokens, Hand *hand) {
    for (int i = 0; i < 5; i++) {
        hand->cards[i].value = card_value(tokens[i][0]);
        hand->cards[i].suit = tokens[i][1];
    }
    qsort(hand->cards, 5, sizeof(Card), cmp);
}

// Evaluate hand: returns a 64-bit integer encoding hand rank and tiebreakers
unsigned long long eval_hand(Hand *h) {
    int v[15] = {0}, s[4] = {0}, vals[5];
    int flush = 1, straight = 1, maxv = h->cards[0].value;
    for (int i = 0; i < 5; i++) {
        v[h->cards[i].value]++;
        vals[i] = h->cards[i].value;
        if (i > 0 && h->cards[i].suit != h->cards[0].suit) flush = 0;
        if (i > 0 && h->cards[i].value != h->cards[i-1].value - 1) straight = 0;
    }
    // Special case: 5 4 3 2 A
    if (vals[0]==14 && vals[1]==5 && vals[2]==4 && vals[3]==3 && vals[4]==2) straight = 1, maxv=5;
    int counts[5] = {0}, n=0;
    for (int i=2; i<=14; i++) if (v[i]) counts[n++] = v[i];
    // Encode hand: rank, then card values for tiebreak
    unsigned long long score = 0;
    if (straight && flush && maxv==14) score = 9ULL<<28; // Royal Flush
    else if (straight && flush) score = (8ULL<<28) | (maxv<<20);
    else if (counts[0]==4) { // Four of a Kind
        int four=0, kicker=0;
        for (int i=2;i<=14;i++) if (v[i]==4) four=i; else if (v[i]) kicker=i;
        score = (7ULL<<28) | (four<<20) | (kicker<<16);
    }
    else if (counts[0]==3 && counts[1]==2) { // Full House
        int three=0, pair=0;
        for (int i=2;i<=14;i++) if (v[i]==3) three=i; else if (v[i]==2) pair=i;
        score = (6ULL<<28) | (three<<20) | (pair<<16);
    }
    else if (flush) score = (5ULL<<28) | (vals[0]<<20) | (vals[1]<<16) | (vals[2]<<12) | (vals[3]<<8) | (vals[4]<<4);
    else if (straight) score = (4ULL<<28) | (maxv<<20);
    else if (counts[0]==3) { // Three of a Kind
        int three=0, kickers[2], k=0;
        for (int i=14;i>=2;i--) if (v[i]==3) three=i;
        for (int i=14;i>=2;i--) if (v[i]==1) kickers[k++]=i;
        score = (3ULL<<28) | (three<<20) | (kickers[0]<<16) | (kickers[1]<<12);
    }
    else if (counts[0]==2 && counts[1]==2) { // Two Pairs
        int high=0, low=0, kicker=0;
        for (int i=14;i>=2;i--) if (v[i]==2) { if (!high) high=i; else low=i; }
        for (int i=14;i>=2;i--) if (v[i]==1) kicker=i;
        score = (2ULL<<28) | (high<<20) | (low<<16) | (kicker<<12);
    }
    else if (counts[0]==2) { // One Pair
        int pair=0, kickers[3], k=0;
        for (int i=14;i>=2;i--) if (v[i]==2) pair=i;
        for (int i=14;i>=2;i--) if (v[i]==1) kickers[k++]=i;
        score = (1ULL<<28) | (pair<<20) | (kickers[0]<<16) | (kickers[1]<<12) | (kickers[2]<<8);
    }
    else // High Card
        score = (0ULL<<28) | (vals[0]<<20) | (vals[1]<<16) | (vals[2]<<12) | (vals[3]<<8) | (vals[4]<<4);
    return score;
}

int main(int argc, char *argv[]) {
    FILE *f = fopen(INPUT_FILE, "r");
    if (!f) { perror("File open failed"); return 1; }
    char line[64], *tokens[10];
    int p1wins = 0;
    while (fgets(line, sizeof(line), f)) {
        int t = 0;
        tokens[t++] = strtok(line, " \n");
        while (tokens[t-1] && t < 10) tokens[t++] = strtok(NULL, " \n");
        if (t < 10) continue;
        Hand h1, h2;
        parse_hand(tokens, &h1);
        parse_hand(tokens+5, &h2);
        unsigned long long s1 = eval_hand(&h1), s2 = eval_hand(&h2);
        if (s1 > s2) p1wins++;
    }
    fclose(f);
    printf("%d\n", p1wins);
    return EXIT_SUCCESS;
}