#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux_listsort.h"
#include "queue.h"


/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Free the storage used by an element */
#define free_element(e) \
do {                \
    free(e->value); \
    free(e);        \
} while (0)

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_head = malloc(sizeof(struct list_head));
    if (!new_head)
        return NULL;

    new_head->next = new_head;
    new_head->prev = new_head;

    return new_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *element, *safe = NULL;

    list_for_each_entry_safe(element, safe, head, list) {
        list_del(&element->list);
        free_element(element);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    INIT_LIST_HEAD(&new_element->list);

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add(&new_element->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    INIT_LIST_HEAD(&new_element->list);

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add_tail(&new_element->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);
    if (sp && entry->value) {
        strncpy(sp, entry->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);

    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);
    if (entry->value && sp) {
        strncpy(sp, entry->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);

    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int count = 0;

    for (struct list_head *node = head->next; node != head; node = node->next)
        count++;

    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next, *fast = slow;

    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    /* Slow is the middle node */
    list_del(slow);
    free_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    element_t *prev = list_first_entry(head, element_t, list);
    struct list_head *prev_node = head->next;
    struct list_head *node = prev_node->next, *next_node;
    bool is_dup = false;

    while (node != head) {
        next_node = node->next;
        element_t *e = list_entry(node, element_t, list);
        if (!strcmp(prev->value, e->value)) {
            list_del(node);
            free_element(e);
            is_dup = true;
        } else {
            if (is_dup) {
                list_del(prev_node);
                free_element(prev);
                is_dup = false;
            }
            prev = e;
            prev_node = node;
        }
        node = next_node;
    }

    if (is_dup) {
        list_del(prev_node);
        free_element(prev);
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *node = head->next;

    while (node != head && node->next != head) {
        struct list_head *next_node = node->next, *tmp;
        /* Swap the nodes */
        node->prev->next = next_node;
        next_node->next->prev = node;
        node->next = next_node->next;
        next_node->next = node;
        tmp = node->prev;
        node->prev = next_node;
        next_node->prev = tmp;
        /* Move to the next pair */
        node = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *node = head->next, *nextnode;

    /* Swap next and prev pointers of all the nodes */
    while (node != head) {
        nextnode = node->next;
        node->next = node->prev;
        node->prev = nextnode;
        /* Move to the next node */
        node = nextnode;
    }
    node = head->next;
    head->next = head->prev;
    head->prev = node;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k <= 1)
        return;

    int count = q_size(head);

    if (count < k)
        return;

    int round = count / k;
    struct list_head *node = head;
    for (int i = 0; i < round; i++) {
        struct list_head *cur = node->next, *next, *tmp;
        /* Reverse the nodes */
        for (int j = 0; j < k; j++) {
            next = cur->next;
            tmp = cur->next;
            cur->next = cur->prev;
            cur->prev = tmp;
            cur = next;
        }
        /* Swap the pointers */
        next->prev->prev = node;
        node->next->next = next;
        tmp = next->prev;
        next->prev = node->next;
        node->next = tmp;
        /* Move to the next group */
        node = next->prev;
    }
}

void merge(struct list_head *head, struct list_head *l, struct list_head *r)
{
    /* Merge the two sorted lists left and right into head */
    while (!list_empty(l) && !list_empty(r)) {
        char *l_val = list_first_entry(l, element_t, list)->value;
        char *r_val = list_first_entry(r, element_t, list)->value;
        struct list_head *node = strcmp(l_val, r_val) <= 0 ? l->next : r->next;
        list_move_tail(node, head);
    }

    /* Move the remaining elements to head */
    list_splice_tail_init(l, head);
    list_splice_tail_init(r, head);
}

void merge_sort_ascend(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head left = {&(left), &(left)};
    struct list_head right = {&(right), &(right)};

    struct list_head *slow = head->next, *fast = slow;

    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    /* Split the list into two halves */
    list_cut_position(&left, head, slow);
    list_splice_tail_init(head, &right);

    merge_sort_ascend(&left);
    merge_sort_ascend(&right);
    merge(head, &left, &right);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    merge_sort_ascend(head);

    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    struct list_head *node = head->prev, *prev_node;
    const char *min = list_entry(node, element_t, list)->value;
    node = node->prev;

    while (node != head) {
        prev_node = node->prev;
        element_t *e = list_entry(node, element_t, list);
        if (strcmp(e->value, min) < 0)
            min = e->value;
        else {
            list_del(node);
            free_element(e);
        }
        node = prev_node;
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    struct list_head *node = head->prev, *prev_node;
    const char *max = list_entry(node, element_t, list)->value;
    node = node->prev;

    while (node != head) {
        prev_node = node->prev;
        element_t *e = list_entry(node, element_t, list);
        if (strcmp(e->value, max) > 0)
            max = e->value;
        else {
            list_del(node);
            free_element(e);
        }
        node = prev_node;
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    queue_contex_t *first_qc = list_first_entry(head, queue_contex_t, chain);
    struct list_head *cur_chain = (&(first_qc->chain))->next;

    for (; cur_chain != head; cur_chain = cur_chain->next) {
        queue_contex_t *cur_qc = list_entry(cur_chain, queue_contex_t, chain);
        list_splice_init(cur_qc->q, first_qc->q);
        first_qc->size += cur_qc->size;
        cur_qc->size = 0;
    }

    q_sort(first_qc->q, descend);

    return first_qc->size;
}
