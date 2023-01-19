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

#include <assert.h>
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
 * A method for editing the member of the reviewer structure that holds a username.
 * param: rv, the struct representing the reviewer in question
 * param: username, a string with the username intended to replace the existing one
**/
void change_username(struct reviewer *rv, char* username)
{
    // Check this reviewer has write privileges
    // and replace the username if so,
    // otherwise display a message and return.
    if(can_write(rv))
        rv->username = username;
    else
        printf("You don't have permission to change this username.\n");

}

/**
 * A method for editing the member of the reviewer structure that holds a real name.
 * param: rv, the struct representing the reviewer in question
 * param: realname, a string with the realname intended to replace the existing one
**/
void change_realname(struct reviewer *rv, char* realname)
{
    // Check this reviewer has write privileges
    // and replace the realname if so,
    // otherwise display a message and return.
    if(can_write(rv))
        rv->realname = realname;
    else
        printf("You don't have permission to change this real name.\n");

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
    #ifdef __CHERI_PURE_CAPABILITY__
        // If this is read only, it will crash.
        // To test this in a CHERI environment,
        // we won't do a weak check of read-only privileges here.
        rv->privatereview = privatereview;
    #else
        if(can_write(rv))
            rv->privatereview = privatereview;
        else
            printf("You don't have permission to change this private review.\n");
    #endif
}

/**
 * A method for editing the member of the reviewer structure that holds a public review.
 * param: rv, the struct representing the reviewer in question
 * param: publicreview, a string with the public review intended to replace the existing one
**/
void change_publicreview(struct reviewer *rv, char* publicreview)
{

    if(can_write(rv))
        rv->publicreview = publicreview;
    else
        printf("You don't have permission to change this public review.\n");
}

/**
 * A method for attempting to change the private review to something more flattering.
 * param: rv, the struct representing the reviewer in question.
 * param: reqsz, a size_t representing the amount of memory to allocate for the new review.
**/
void try_improve_privatereview(struct reviewer *rv, size_t reqsz, bool b_weak) {
    printf("\nAttempting to change the private review.\n");
    #ifdef __CHERI_PURE_CAPABILITY__
        if((can_write(rv) == false) && (b_weak == true)) {
            // If the reviewer doesn't have write privileges, 
            // the change will not be made.
            printf("You do not have permission to change the private review.\n");
            return;
        } else {
        
            // The more flattering private review.
            char *newprivatereview = malloc(reqsz);
            strcpy(newprivatereview, "5 ***** strong accept. This author deserves two Nobels and an ice cream.\n");
            // if the reviewer is set to read-only, this will crash.
            change_privatereview(rv, newprivatereview);
        }
    #else
        // The more flattering private review.
        char *newprivatereview = malloc(reqsz);
        strcpy(newprivatereview, "5 ***** strong accept. This author deserves two Nobels and an ice cream.\n");
        // if the reviewer is set to read-only, this will crash.
        change_privatereview(rv, newprivatereview);
    #endif
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

/**
 * Get the actual length of a string being assigned to a member.
 * param: userdata, a char * to be assigned to a member of the reviewer struct
 * param: explen, an int representing the allocated length
 * return: actsize, a size_t representing the actual size of the string
 **/
size_t actual_input_length(char *userdata, int explen) 
{
    // Now establish the actual length of the input
    size_t actsz = (size_t)strlen(userdata);
    // we expect something between 1 and the expected length
    assert(actsz > 0);
    assert(actsz <= explen);
    
    return actsz;
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