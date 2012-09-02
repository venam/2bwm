#include <stdlib.h>
#include <stdio.h>
#include "list.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#ifdef DEBUG
#define PDEBUG(Args...) \
  do { fprintf(stderr, "mcwm: "); fprintf(stderr, ##Args); } while(0)
#define D(x) x
#else
#define PDEBUG(Args...)
#define D(x)
#endif

/*
 * Move element in item to the head of list mainlist.
 */ 
void movetohead(struct item **mainlist, struct item *item)
{
    if (NULL == item || NULL == mainlist || NULL == *mainlist)
    {
        return;
    }
    
    if (*mainlist == item)
    {
        /* item is NULL or we're already at head. Do nothing. */
        return;
    }

    /* Braid together the list where we are now. */
    if (NULL != item->prev)
    {
        item->prev->next = item->next;
    }

    if (NULL != item->next)
    {
        item->next->prev = item->prev;
    }

    /* Now we'at head, so no one before us. */
    item->prev = NULL;

    /* Old head is our next. */
    item->next = *mainlist;

    /* Old head needs to know about us. */
    item->next->prev = item;
    
    /* Remember the new head. */
    *mainlist = item;
}

/*
 * Create space for a new item and add it to the head of mainlist.
 *
 * Returns item or NULL if out of memory.
 */
struct item *additem(struct item **mainlist)
{
    struct item *item;
    
    if (NULL == (item = (struct item *) malloc(sizeof (struct item))))
    {
        return NULL;
    }
  
    if (NULL == *mainlist)
    {
        /* First in the list. */

        item->prev = NULL;
        item->next = NULL;
    }
    else
    {
        /* Add to beginning of list. */

        item->next = *mainlist;
        item->next->prev = item;
        item->prev = NULL;
    }

    *mainlist = item;
        
    return item;
}

void delitem(struct item **mainlist, struct item *item)
{
    struct item *ml = *mainlist;
    
    if (NULL == mainlist || NULL == *mainlist || NULL == item)
    {
        return;
    }

    if (item == *mainlist)
    {
        /* First entry was removed. Remember the next one instead. */
        *mainlist = ml->next;
    }
    else
    {
        item->prev->next = item->next;

        if (NULL != item->next)
        {
            /* This is not the last item in the list. */
            item->next->prev = item->prev;
        }
    }

    free(item);
}

void freeitem(struct item **list, int *stored,
              struct item *item)
{
    if (NULL == list || NULL == *list || NULL == item)
    {
        return;
    }
    
    if (NULL != item->data)
    {
        free(item->data);
        item->data = NULL;
    }

    delitem(list, item);

    if (NULL != stored)
    {
        (*stored) --;
    }
}

/*
 * Delete all elements in list and free memory resources.
 */ 
void delallitems(struct item **list, int *stored)
{
    struct item *item;
    struct item *next;
    
    for (item = *list; item != NULL; item = next)
    {
        next = item->next;
        free(item->data);
        delitem(list, item);
    }

    if (NULL != stored)
    {
        (*stored) = 0;
    }    
}

void listitems(struct item *mainlist)
{
    struct item *item;
    int i;
    
    for (item = mainlist, i = 1; item != NULL; item = item->next, i ++)
    {
        printf("item #%d (stored at %p).\n", i, (void *)item);
    }
}

#if 0

void listall(struct item *mainlist)
{
    struct item *item;
    int i;

    printf("Listing all:\n");
    
    for (item = mainlist, i = 1; item != NULL; item = item->next, i ++)
    {
        printf("%d at %p: %s.\n", i, (void *)item, (char *)item->data);
        printf("  prev: %p\n", item->prev);
        printf("  next: %p\n", item->next);
    }
}

int main(void)
{
    struct item *mainlist = NULL;
    struct item *item1;
    struct item *item2;
    struct item *item3;
    struct item *item4;
    struct item *item;
    struct item *nextitem;
    int i;
    char *foo1 = "1";
    char *foo2 = "2";
    char *foo3 = "3";
    char *foo4 = "4";
    
    item1 = additem(&mainlist);
    if (NULL == item1)
    {
        printf("Couldn't allocate.\n");
        exit(1);
    }
    item1->data = foo1;
    printf("Current elements:\n");
    listall(mainlist);
    
    item2 = additem(&mainlist);
    if (NULL == item2)
    {
        printf("Couldn't allocate.\n");
        exit(1);
    }
    item2->data = foo2;
    printf("Current elements:\n");    
    listall(mainlist);
    
    item3 = additem(&mainlist);
    if (NULL == item3)
    {
        printf("Couldn't allocate.\n");
        exit(1);
    }
    item3->data = foo3;
    printf("Current elements:\n");    
    listall(mainlist);

    item4 = additem(&mainlist);
    if (NULL == item4)
    {
        printf("Couldn't allocate.\n");
        exit(1);
    }
    item4->data = foo4;
    printf("Current elements:\n");    
    listall(mainlist);

    printf("----------------------------------------------------------------------\n");

    printf("Moving item3 to be after item2\n");
    movetonext(&mainlist, item2, item3);
    printf("Current elements:\n");    
    listall(mainlist);

    printf("----------------------------------------------------------------------\n");

    printf("Moving head! item4 to be after item2\n");
    movetonext(&mainlist, item2, item4);
    printf("Current elements:\n");    
    listall(mainlist);

    printf("----------------------------------------------------------------------\n");
    
    printf("Moving tail! item1 to be after item2\n");
    movetonext(&mainlist, item2, item1);
    printf("Current elements:\n");
    listall(mainlist);

    printf("----------------------------------------------------------------------\n");

    printf("Moving head to be after tail.\n");
    movetonext(&mainlist, item3, item2);
    printf("Current elements:\n");
    listall(mainlist);
    
    printf("Moving all the items after each other.\n");
    /* item3 is tail. work backwards. */
    for (item = mainlist, i = 1;
         item != NULL;
         item = item->next, i ++)
    {
        for (nextitem = item2; nextitem != NULL; nextitem = nextitem->prev)
        {
            movetonext(&mainlist, nextitem, item);
            printf("Current elements:\n");
            listall(mainlist);
        }
    }
    
    printf("----------------------------------------------------------------------\n");

#if 0    
    movetohead(&mainlist, item2);
    printf("Current elements:\n");    
    listall(mainlist);
    
    printf("----------------------------------------------------------------------\n");
#endif
    
    printf("Deleting item stored at %p\n", item3);
    delitem(&mainlist, item3);
    printf("Current elements:\n");    
    listall(mainlist);

    puts("");
    
    delitem(&mainlist, item2);
    printf("Current elements:\n");    
    listall(mainlist);

    puts("");
    
    delitem(&mainlist, item1);
    printf("Current elements:\n");    
    listall(mainlist);

    puts("");

    printf("----------------------------------------------------------------------\n");
               
    exit(0);
}
#endif
