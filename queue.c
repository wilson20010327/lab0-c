#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *node = test_malloc(sizeof(struct list_head));
    /* check whether malloc worked or not */
    if (!node)
        return NULL;
    /* use linux kernal api to init list head*/
    INIT_LIST_HEAD(node);
    return node;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *pos, *n;
    list_for_each_entry_safe (pos, n, l, list)
        q_release_element(pos);
    test_free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    char *content = strdup(s);
    if (!element || !content) {
        test_free(element);
        test_free(content);
        return false;
    }
    element->value = content;
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    char *content = strdup(s);
    if (!element || !content) {
        test_free(element);
        test_free(content);
        return false;
    }
    element->value = content;
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_first_entry(head, element_t, list);
    list_del(&element->list);
    if (!sp)
        return element;
    strncpy(sp, element->value, bufsize);
    sp[bufsize - 1] = '\0';
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_last_entry(head, element_t, list);
    list_del(&element->list);
    if (!sp)
        return element;
    strncpy(sp, element->value, bufsize);
    sp[bufsize - 1] = '\0';
    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *left = head->next, *right = head->prev;
    while (left != right && left != right->prev) {
        left = left->next;
        right = right->prev;
    }

    list_del(right);
    /*delete the whole element after remove from the list */
    element_t *element = list_entry(right, element_t, list);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return 0;
    LIST_HEAD(pending_head);
    element_t *pos, *safe;
    struct list_head *new_head = head->next;
    list_for_each_entry_safe (pos, safe, head, list) {
        if ((&safe->list != head) && !strcmp(pos->value, safe->value))
            /* if match*/
            continue;
        else {
            /* if no match or the end */
            if (&pos->list != new_head) {
                LIST_HEAD(temp_head);
                list_cut_position(&temp_head, new_head->prev, &pos->list);
                list_splice(&temp_head, &pending_head);
            }
            new_head = &safe->list;
        }
    }
    list_for_each_entry_safe (pos, safe, &pending_head, list)
        q_release_element(pos);

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    /* sepcial case of the reverseK*/
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *pos, *safe;
    list_for_each_safe (pos, safe, head) {
        list_move(pos, head);
    }
    return;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    int count = k;
    struct list_head *cur, *safe, *remain_head;
    remain_head = head;
    list_for_each_safe (cur, safe, head) {
        if (--count)
            continue;
        count = k;
        /*create a new head to store k elements that need to reverse*/
        LIST_HEAD(new_head);
        /* cut k element and create a new list*/
        list_cut_position(&new_head, remain_head, cur);
        /* reverse the new list*/
        q_reverse(&new_head);
        /* add it to the origin list */
        list_splice(&new_head, remain_head);
        /* record the remain list need to reverse*/
        remain_head = safe->prev;
    }
}
void q_merge2(struct list_head *head,
              struct list_head *second_head,
              bool descend)
{
    /* merge two list and head will point to the new list */
    if (!head || !second_head)
        return;
    LIST_HEAD(temp);
    while (!list_empty(head) && !list_empty(second_head)) {
        element_t *node1 = list_first_entry(head, element_t, list);
        element_t *node2 = list_first_entry(second_head, element_t, list);
        bool cmp = (strcmp(node1->value, node2->value) <= 0);
        cmp = descend ? !cmp : cmp;
        if (cmp) {
            list_move_tail(&node1->list, &temp);
        } else {
            list_move_tail(&node2->list, &temp);
        }
    }
    /* splice the list in front of the list pointed by head and head point to
     * the new list */
    list_splice(&temp, head);
    if (!list_empty(second_head)) {
        list_splice_tail(second_head, head);
    }
}
/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    /* use merge sort */
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    /* find mid to split*/
    struct list_head *left = head->next, *right = head->prev, *mid;
    while (left != right && left != right->prev) {
        left = left->next;
        right = right->prev;
    }
    mid = right;
    LIST_HEAD(split_head);
    list_cut_position(&split_head, head, mid->prev);
    q_sort(head, descend);
    q_sort(&split_head, descend);
    q_merge2(head, &split_head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    element_t *cur = list_last_entry(head, element_t, list);
    int count = 1;
    LIST_HEAD(pending_head);
    while (cur->list.prev != head) {
        element_t *check_node = list_entry(cur->list.prev, element_t, list);
        bool cmp = (strcmp(cur->value, check_node->value) >= 0);
        if (!cmp) {
            list_move(&check_node->list, &pending_head);
        } else {
            count++;
            cur = check_node;
        }
    }
    element_t *pos, *safe;
    list_for_each_entry_safe (pos, safe, &pending_head, list)
        q_release_element(pos);
    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    element_t *cur = list_last_entry(head, element_t, list);
    int count = 1;
    LIST_HEAD(pending_head);
    while (cur->list.prev != head) {
        element_t *check_node = list_entry(cur->list.prev, element_t, list);
        bool cmp = (strcmp(cur->value, check_node->value) <= 0);
        if (!cmp) {
            list_move(&check_node->list, &pending_head);
        } else {
            count++;
            cur = check_node;
        }
    }
    element_t *pos, *safe;
    list_for_each_entry_safe (pos, safe, &pending_head, list)
        q_release_element(pos);
    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
