struct item
{
    void *data;
    struct item *prev;
    struct item *next;
};

/*
 * Move element in item to the head of list mainlist.
 */
void movetohead(struct item **mainlist, struct item *item);

/*
 * Create space for a new item and add it to the head of mainlist.
 *
 * Returns item or NULL if out of memory.
 */
struct item *additem(struct item **mainlist);

/*
 * Delete item from list mainlist.
 */ 
void delitem(struct item **mainlist, struct item *item);

/*
 * Free any data in current item and then delete item. Optionally
 * update number of items in list if stored != NULL.
 */
void freeitem(struct item **list, int *stored,
              struct item *item);

/*
 * Delete all items in list. Optionally update number of items in list
 * if stored != NULL.
 */
void delallitems(struct item **list, int *stored);

/*
 * Print all items in mainlist on stdout.
 */ 
void listitems(struct item *mainlist);
