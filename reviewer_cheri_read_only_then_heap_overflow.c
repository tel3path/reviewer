/** 
 * This version can be run in a CHERI environment to show the effects of 
 * using capabilities to set a data structure to read-only,
 * then without directly trying to edit the read-only members of the structure
 * (which would crash in a CHERI environment; see reviewer_read_only.c),
 * attempt a heap buffer overflow.
 * 
 * This file is based on the read-only employee example at
 * https://github.com/capablevms/cheri-examples/blob/master/employee/include/employee.h.
 * In a CHERI environment, it uses capabilities to enforce read-only privileges.
 * 
 * The program goes on to attempt a buffer overflow attack,
 * which is based on the CHERI adversarial example of a heap overflow at 
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

    printf("smallsz=%zx, CRRL(smallsz)=%zx\n", smallsz,
        __builtin_cheri_round_representable_length(smallsz));
    printf("biggersz=%zx, CRRL(biggersz)=%zx\n", biggersz,
        __builtin_cheri_round_representable_length(biggersz));
    printf("username=%#p realname=%#p diff=%tx\n", username, realname, realname - username);
    printf("realname=%#p privatereview=%#p diff=%tx\n", realname, privatereview, privatereview - realname);
    printf("privatereview=%#p publicreview=%#p diff=%tx\n", privatereview, publicreview, publicreview - privatereview);

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

    // Protect the reviewer by setting
    // the relevant struct members (in this case all of them)
    // to read-only.           
    struct reviewer *ro_reviewer = set_read_only(&reviewer);
    assert((cheri_perms_get(ro_reviewer) & CHERI_PERM_STORE) == 0);
    print_details(ro_reviewer);

    printf("\nThe struct has been set to read-only.\n");
    fflush(stdout);
    
    // Now we get to the attack part.
    // Hopefully this will overwrite the beginning of the reviewer's real name.
    // In a CHERI environment this should crash.
    printf("\nOverflowing reviewer username by 1\n");
    memset(ro_reviewer->username + smallsz, 'A', 2);
    
    // Now we can see the changes, if any.
    print_details(ro_reviewer);
    
    // Overflow the username even more, which in a CHERI
    // environment should crash if it hasn't already.
    const size_t oversz = ro_reviewer->realname - ro_reviewer->username + 2 - smallsz;
    printf("\nOverflowing reviewer username by %zx\n", oversz);
    memset(ro_reviewer->username + smallsz, 'A', oversz);
    
    print_details(ro_reviewer);
    
    // Now try overflowing the private review, with its larger size and wider bounds.
    // In a non-CHERI environment this might not result in any visible changes.
    printf("\nOverflowing private remarks by 1\n");
    memset(ro_reviewer->privatereview + biggersz, 'A', 2);
    
    print_details(ro_reviewer);
    
    // Now try overflowing the private review even more.
    // In a non-CHERI environment this might change the start
    // of the public review from "While" to "!hile"
    const size_t overbigger = ro_reviewer->publicreview - ro_reviewer->privatereview + 2 - biggersz;
    printf("\nOverflowing private review by %zx\n", overbigger);
    // TODO refactor this to something less clumsy
    if(overbigger >= 1)
        memset(ro_reviewer->privatereview + biggersz, 'A', 1);
    if(overbigger >= 2)
        memset(ro_reviewer->privatereview + biggersz+1, '+', 1);
    if(overbigger >= 3)
        memset(ro_reviewer->privatereview + biggersz+2, '+', 1);
    if(overbigger >= 4)
        memset(ro_reviewer->privatereview + biggersz+3, '\n', 1);
    if(overbigger >= 5)
        memset(ro_reviewer->privatereview + biggersz+4, '!', overbigger-5);
    
    // We can see all the changes we have made, if any
    print_details(ro_reviewer);

    return 0;
}

