/**
 * Reviewer struct and auxiliary functions to display reviewer's details,
 * change the content, and operate on the struct's permissions.
 *
 * This is based on the employee example at
 * https://github.com/capablevms/cheri-examples/blob/master/employee/include/employee.h
**/

#ifdef __CHERI_PURE_CAPABILITY__
#include "../../include/common.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * TODO separate the reviewer from the review,
 * as the current design does not make sense.
 * TODO It also makes no logical sense that they could write a review
 * and then not edit any of it. It's other reviews they shouldn't be able to see.
**/
struct reviewer
{
    // The reviewer's handle in the reviewing system.
    char *username;
    // The reviewer's real name (which they would expect to be confidential)
    char *realname;
    // The reviewer's private remarks, which they would expect to be visible
    // only to the committee
    char *privatereview;
    // The review, which should be visible only to the committee and the authors
    char *publicreview;
    // The permissions this reviewer has (r, rw, rwx and so on)
    char *privileges;
};

/**
 * Check this reviewer's privileges.
 * param: reviewer, the struct with the details of the reviewer whose privileges we are checking
 * return: true if the reviewer has write privileges, false otherwise.
**/
bool can_write(struct reviewer *rv) {
    // assume they have write privileges until we know otherwise
    bool canwrite = true;
    
    // a rigorous protocol of looking for the letter w in their privileges
    char *pt = strchr(rv->privileges, 'w');
    // no letter w, no write privileges
    if(pt == NULL)
        return false;

    // if we reached this line, they have write privileges
    return canwrite;
}

/**
 * A method for editing the member of the reviewer structure that holds a private review.
 * param: rv, the struct representing the reviewer in question
 * param: privatereview, a string with the private review intended to replace the existing one
**/
void change_privatereview(struct reviewer *rv, char* privatereview)
{
    // Check this reviewer has write privileges
    // and replace the private review if so,
    // otherwise display a message and return.
    if(can_write(rv))
        rv->privatereview = privatereview;
    else
        printf("You don't have permission to change this private review.\n");
}

/**
 * A method for editing the member of the reviewer structure that holds a public review.
 * TODO check privileges before allowing this.
 * param: rv, the struct representing the reviewer in question
 * param: publicreview, a string with the public review intended to replace the existing one
**/
void change_publicreview(struct reviewer *rv, char* publicreview)
{
    // Replace the public review with the new one
    rv->publicreview = publicreview;
}

/**
 * Print all the member values for a given review.
 * TODO check privileges before allowing this.
 * param: rv, the struct representing the reviewer in question
**/
void print_details(struct reviewer *rv)
{
    printf("Reviewer Username: %s \n", rv->username);
    printf("Reviewer's Real Name: %s \n", rv->realname);
    printf("Private Review: %s\n", rv->privatereview);
    printf("Public Review: %s\n", rv->publicreview);
    printf("Reviewer's User Privileges: %s\n", rv->privileges);
    fflush(stdout);
}

#ifdef __CHERI_PURE_CAPABILITY__
/**
 * Traverse the struct to set the permissions.
 * In this case we set all the members to read-only.
 * param: rv, the struct representing the reviewer to display
 * return: a reviewer with privileges restricted to read-only
 **/
struct reviewer *set_read_only(struct reviewer *rv)
{
    rv->username = (char *) cheri_perms_and(rv->username, CHERI_PERM_LOAD);
    rv->realname = (char *) cheri_perms_and(rv->realname, CHERI_PERM_LOAD);
    rv->privatereview = (char *) cheri_perms_and(rv->privatereview, CHERI_PERM_LOAD);
    rv->publicreview = (char *) cheri_perms_and(rv->publicreview, CHERI_PERM_LOAD);
    rv->privileges = (char *) cheri_perms_and(rv->privileges, CHERI_PERM_LOAD);
    return (struct reviewer *) cheri_perms_and(rv, CHERI_PERM_LOAD | CHERI_PERM_LOAD_CAP);
}
#endif