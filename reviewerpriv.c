/** 
 * This version can be run in a non-CHERI environment to show the difference
 * between the two environments. 
 * 
 * This is based on the full-privilege employee example at
 * https://github.com/capablevms/cheri-examples/blob/master/employee/include/employee.h
 * and on the CHERI adversarial example of a heap overflow at 
 * https://ctsrd-cheri.github.io/cheri-exercises/exercises/buffer-overflow-heap/index.html.
 *
 */

#include "include/reviewer.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * TODO decompose this as it's just one long main() right now.\
 *
 * The motivating example is an academic review which should be
 * confidential to the committee, with only a subset of the data
 * readable by the authors.
 *
**/
int main()
{
    // We will prompt the user to input these.
    // TODO separate the review from the reviewer.
    // TODO create a superuser who can see all reviews and edit them where appropriate.
    // TODO create an ordinary reviewer who can create reviews,
    // see their own reviews, and edit some but not all of the members.
    // TODO create an author who can read the public reviews of their own papers,
    // but not edit them.
    // TODO create a relation between review and author.
    char *username, *realname, *privatereview, *publicreview;
    // Each user gets read privileges by default.
    // TODO create a superuser with rwx privileges
    char *privileges = "r";
    
    // Our new reviewer with read only privileges.
    struct reviewer reviewer;
    
    // We will allocate smaller amounts of memory for names,
    // and larger amounts of memory for reviews.
    size_t smallsz = 0x20;
    size_t biggersz = 0x1001;
    
    // Prompt the user for input.
    printf("Create your review!\n");
    username = malloc(smallsz);
    printf("Enter your user name: \n");
    fgets(username, smallsz, stdin);
    
    // Now establish the actual length of the input
    // TODO decompose this repeated code
    size_t szn = (size_t)strlen(username);
    // we expect something between 1 and 32
    assert(szn > 0);
    assert(szn <= smallsz);
    printf("size is %zu\n", szn);
    // trim any newlines off the end of the input
    if(username[szn-1] == '\n') {
        username[szn-1] = '\0';
        szn = (size_t)strlen(username);
    }
    
    // Now we prompt them for their real name:
    // TODO should only be visible to the reviewer
    // and the committee
    realname = malloc(smallsz);
    printf("Enter your real name: \n");
    // Get and save the text
    fgets(realname, smallsz, stdin);
    
    // Establish the actual length of the input
    // TODO decompose this repeated code
    size_t szp = (size_t)strlen(realname);
    // we expect something between 1 and 32
    assert(szp > 0);
    assert(szp <= smallsz);
    printf("size is %zu\n", szp);
    // trim any newlines off the end of the input
    if(realname[szp-1] == '\n') {
        realname[szp-1] = '\0';
        szp = (size_t)strlen(realname);
    }
    
    // Now we prompt them for their private review:
    privatereview = malloc(biggersz);
    printf("Enter your private remarks: \n");

    // Get and save the text
    fgets(privatereview, biggersz, stdin);
    size_t szprr = (size_t)strlen(privatereview);
    // we expect something between 1 and 4097
    assert(szprr > 0);
    assert(szprr <= biggersz);
    printf("size is %zu\n", szprr);
    if(privatereview[szprr-1] == '\n') {
        privatereview[szprr-1] = '\0';
        szprr = (size_t)strlen(privatereview);
    }

    // Now we prompt them for their public review:
    publicreview = malloc(biggersz);
    // public review
    printf("Enter your public review: \n");
    // Get and save the text
    fgets(publicreview, biggersz, stdin);
    size_t szpur = (size_t)strlen(publicreview);
    // we expect something between 1 and 4097
    assert(szpur > 0);
    assert(szpur <= biggersz);
    printf("size is %zu\n", szpur);
    if(publicreview[szpur-1] == '\n') {
        publicreview[szpur-1] = '\0';
        szpur = (size_t)strlen(publicreview);
    }

    
// If we are in a CHERI purecap environment, show the representable length
// as well as the distance between the addresses of the members.
// If we are not in a CHERI purecap environment, just show the distances.
#ifdef __CHERI_PURE_CAPABILITY__
    printf("smallsz=%zx, CRRL(smallsz)=%zx\n", smallsz,
        __builtin_cheri_round_representable_length(smallsz));
    printf("biggersz=%zx, CRRL(biggersz)=%zx\n", biggersz,
        __builtin_cheri_round_representable_length(biggersz));
    printf("username=%#p realname=%#p diff=%tx\n", username, realname, realname - username);
    printf("realname=%#p privatereview=%#p diff=%tx\n", realname, privatereview, privatereview - realname);
    printf("privatereview=%#p publicreview=%#p diff=%tx\n", privatereview, publicreview, publicreview - privatereview);
#else
    printf("username=%#p realname=%#p diff=%tx\n", username, realname, realname - username);
    printf("realname=%#p privatereview=%#p diff=%tx\n", realname, privatereview, privatereview - realname);
    printf("privatereview=%#p publicreview=%#p diff=%tx\n", privatereview, publicreview, publicreview - privatereview);
#endif

    // These addresses need to be close to each other in order
    // to make the point we're trying to make.
    // If these asserts fail, rerun until they pass.
    // TODO rework the asserts so that they pass consistently
    // in each environment. 
    assert((size_t)(username + smallsz) <= (size_t)realname); 
    assert((size_t)(privatereview + biggersz) <= (size_t)publicreview);
    // TODO review the assumptions underlying these asserts,
    // as they don't seem to hold any more.
    //assert((ptraddr_t)(realname + szp + szp/2) > (ptraddr_t)privatereview); 
    //assert((ptraddr_t)(privatereview + szprr + szprr/2) > (ptraddr_t)publicreview);
    
    // Echo the user's input back to them.
    printf("Username: %s\n", username);
    printf("Real name: %s\n", realname);
    printf("Private review: %s\n", privatereview);
    printf("Public review: %s\n", publicreview);
    
    // Assign the input to the reviewer.
    reviewer.username = username;
    reviewer.realname = realname;
    reviewer.privatereview = privatereview;
    reviewer.publicreview = publicreview;
    reviewer.privileges = privileges;
    
#ifdef __CHERI_PURE_CAPABILITY_
    if(!can_write(&reviewer)) {
        struct reviewer *ro_reviewer = set_read_only(&reviewer);
        assert((cheri_perms_get(ro_reviewer) & CHERI_PERM_STORE) == 0);
        print_details(ro_reviewer);
        char *newprivatereview = malloc(biggersz);
        strcpy(newprivatereview, "5 ***** strong accept. This author deserves two Nobels and an ice cream.\n");
        printf("%s", newprivatereview);

        printf(
            "\nThe struct is read-only so trying to change the review to \" %s \" will make the program "
            "crash...\n",
            newprivatereview);

        fflush(stdout);
        printf("Attempting to change the private review.\n");
        change_privatereview(ro_reviewer, newprivatereview);
        print_details(ro_reviewer);
    }
#endif
    
    // Now we will try to edit the private review,
    // replacing it with a more flattering one.
    // TODO: create another reviewer so that this action
    // actually makes sense. Right now the user is forbidden
    // to edit their own review.
    printf("\nAttempting to change the private review.\n");
    // The more flattering private review.
    char *newprivatereview = malloc(biggersz);
    strcpy(newprivatereview, "5 ***** strong accept. This author deserves two Nobels and an ice cream.\n");
    // If the reviewer doesn't have write privileges, 
    // the change will not be made.
    change_privatereview(&reviewer, newprivatereview);

    // Now we get to the attack part.
    // Hopefully this will overwrite the beginning of the reviewer's real name.
    // In a CHERI environment this might crash.
    // In a non-CHERI environment this might not result in any visible changes.
    printf("\nOverflowing reviewer username by 1\n");
    memset(reviewer.username + smallsz, 'A', 2);
    
    // Now we can see the changes, if any.
    print_details(&reviewer);
    
    // Overflow the username even more, which in a CHERI
    // environment should crash if it hasn't already.
    // In a non-CHERI environment this can change "Miss Trunchbull" to "AAss Trunchbull"
    const size_t oversz = reviewer.realname - reviewer.username + 2 - smallsz;
    printf("\nOverflowing reviewer username by %zx\n", oversz);
    memset(reviewer.username + smallsz, 'A', oversz);
    
    print_details(&reviewer);
    
    // Now try overflowing the private review, with its larger size and wider bounds.
    // In a non-CHERI environment this might not result in any visible changes.
    printf("\nOverflowing private remarks by 1\n");
    memset(reviewer.privatereview + biggersz, 'A', 2);
    
    print_details(&reviewer);
    
    // Now try overflowing the private review even more.
    // In a non-CHERI environment this might change the start
    // of the public review from "While" to "!hile"
    const size_t overbigger = reviewer.publicreview - reviewer.privatereview + 2 - biggersz;
    printf("\nOverflowing private review by %zx\n", overbigger);
    // TODO refactor this to something less clumsy
    if(overbigger >= 1)
        memset(reviewer.privatereview + biggersz, 'A', 1);
    if(overbigger >= 2)
        memset(reviewer.privatereview + biggersz+1, '+', 1);
    if(overbigger >= 3)
        memset(reviewer.privatereview + biggersz+2, '+', 1);
    if(overbigger >= 4)
        memset(reviewer.privatereview + biggersz+3, '\n', 1);
    if(overbigger >= 5)
        memset(reviewer.privatereview + biggersz+4, '!', overbigger-5);
    
    // We can see all the changes we have made, if any
    print_details(&reviewer);

    return 0;
}
