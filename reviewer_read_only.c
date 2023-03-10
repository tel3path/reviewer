/** 
 * This version can be run in both a CHERI and a non-CHERI environment to show the 
 * difference between the two environments. 
 * 
 * This file is based on the read-only employee example at
 * https://github.com/capablevms/cheri-examples/blob/master/employee/include/employee.h.
 * In a CHERI environment, it uses capabilities to enforce read-only privileges.
 * Any attempt to write data to the members of the reviewer struct that have
 * been write-protected will crash the program.
 * In a non-CHERI environment, these privileges are weakly enforced and the program
 * will not crash, but display a message and continue.
 * 
 * The program goes on to attempt a buffer overflow attack,
 * which is based on the CHERI adversarial example of a heap overflow at 
 * https://ctsrd-cheri.github.io/cheri-exercises/exercises/buffer-overflow-heap/index.html.
 *
 * In a CHERI environment, this attack code will be unreachable.
 * (To see the effect of this attack on a CHERI read-only data structure that 
 * hasn't been crashed, look at reviewer_cheri_read_only_then_heap_overflow.c.)
 *
 * In a non-CHERI environment, this attack should succeed in corrupting some of the data.
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
    
    // Populate the members of the reviewer struct.
    username = malloc(smallsz);
    strcpy(username, "Reviewer 2");
    
    // Now establish the actual length of the string
    size_t szn = actual_input_length(username, smallsz);
    printf("username size is %zu\n", szn);
    
    // Now we prompt them for their real name:
    // TODO should only be visible to the reviewer
    // and the committee
    realname = malloc(smallsz);
    strcpy(realname, "Baba Yaga");
    
    // Establish the actual length of the string
    size_t szp = actual_input_length(realname, smallsz);
    printf("realname size is %zu\n", szp);
    
    // The private review, only to be seen by committee members
    privatereview = malloc(biggersz);
    strcpy(privatereview, "I cannot believe I read this appalling piece of dreck from start to finish. The authors should be ashamed, and I hope I get an opportunity to tell them so to their faces.");
    
    // establish the actual length of the string
    size_t szprr = actual_input_length(privatereview, biggersz);
    printf("privatereview size is %zu\n", szprr);

    // The public review, only to be seen by committee members and the authors
    publicreview = malloc(biggersz);
    strcpy(publicreview, "I am a little unclear as to the contribution. I think the authors could strengthen their case considerably if they conducted an RCT. Weak reject.");
    
    // establish the actual length of the string
    size_t szpur = actual_input_length(publicreview, biggersz);
    printf("publicreview size is %zu\n", szpur);

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
    //assert((size_t)(realname + szp + szp/2) > (size_t)privatereview); 
    // TODO redefine the assert, as the assumption doesn't hold
    //assert((size_t)(privatereview + szprr + szprr/2) > (size_t)publicreview);
    
    // Assign the member values to the reviewer.
    reviewer.username = username;
    reviewer.realname = realname;
    reviewer.privatereview = privatereview;
    reviewer.publicreview = publicreview;
    reviewer.privileges = privileges;

// If we are in a CHERI environment, protect the reviewer by setting
// the relevant struct members (in this case all of them)
// to read-only.    
#ifdef __CHERI_PURE_CAPABILITY__
    if(can_write(&reviewer) == false) {        
        struct reviewer *ro_reviewer = set_read_only(&reviewer);
        assert((cheri_perms_get(ro_reviewer) & CHERI_PERM_STORE) == 0);
        print_details(ro_reviewer);

        printf("\nThe struct is read-only so trying to change the review will make the program crash...\n");
        fflush(stdout);
        
        try_improve_privatereview(ro_reviewer, biggersz, false);
        
        print_details(ro_reviewer);
    }
# else
    // Try editing the private review in a non-CHERI environment,
    // by simply checking the write privileges and refusing if none are found.
    // We can then go on to 
    // TODO: create another reviewer so that this action
    // actually makes sense. Right now the user is forbidden
    // to edit their own review.
    try_improve_privatereview(&reviewer, biggersz, false);
    print_details(&reviewer);
#endif

    // This code will be unreachable in a CHERI environment,
    // but it is included to make the point.
    
    // Now we get to the attack part.
    // Hopefully this will overwrite the beginning of the reviewer's real name.
    // In a CHERI environment this might crash (if it were reachable).
    // In a non-CHERI environment this might not result in any visible changes.
    printf("\nOverflowing reviewer username by 1\n");
    memset(reviewer.username + smallsz, 'A', 2);
    
    // Now we can see the changes, if any.
    print_details(&reviewer);
    
    // Overflow the username even more, which in a CHERI
    // environment should crash if it hasn't already.
    // In a non-CHERI environment this can change "Baba Yaga" to "AAba Yaga"
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

