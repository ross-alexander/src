typedef unsigned int word;
typedef word node_t;

typedef void* nexthop_t;

typedef struct entryrec *entry_t;
struct entryrec {
   word data;          /* the routing entry */
   int len;            /* and its length */
   nexthop_t nexthop;  /* the corresponding next-hop */
   int pre;            /* this auxiliary variable is used in the */
};                     /* construction of the final data structure */

typedef struct baserec *base_t;
struct baserec {
   word str;    /* the routing entry */
   int len;     /* and its length */
   int pre;     /* pointer to prefix table, -1 if no prefix */
   int nexthop; /* pointer to next-hop table */
};

typedef struct { /* compact version of above */
   word str;
   int len;
   int pre;
   int nexthop;
} comp_base_t;

/* prefix vector */

typedef struct prerec *pre_t;
struct prerec {
   int len;     /* the length of the prefix */
   int pre;     /* pointer to prefix, -1 if no prefix */
   int nexthop; /* pointer to nexthop table */
};

typedef struct { /* compact version of above */
   int len;
   int pre;
   int nexthop;
} comp_pre_t; 


typedef struct routtablerec *routtable_t;
struct routtablerec {
   node_t *trie;         /* the main trie search structure */
   int triesize;
   comp_base_t *base;    /* the base vector */
   int basesize;
   comp_pre_t *pre;      /* the prefix vector */
   int presize;
   nexthop_t *nexthop;   /* the next-hop table */
   int nexthopsize;
};

extern routtable_t buildrouttable(entry_t entry[], int nentries, double fillfact, int rootbranch, int verbose);
extern nexthop_t find(word s, routtable_t t);
extern void routtablestat(routtable_t t, int verbose);

