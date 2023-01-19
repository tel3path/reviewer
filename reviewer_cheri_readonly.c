/* 
 * 
 */

#include "include/reviewer.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	// These will take user input of a username and real name in plain text,	
	// These will be saved in our reviewer's profile.
	char *username, *realname, *privatereview, *publicreview;
	// Each user gets read permissions by default.
	char *privileges = "r";
    
    // Our new reviewer with read only permissions.
    struct reviewer reviewer;
    
    size_t smallsz = 0x20;
    size_t biggersz = 0x1001;
    
    // Prompt the user for input, giving them more than enough space to answer.
    printf("Create your review!\n");
    username = malloc(smallsz);
	printf("Enter your user name: \n");
	fgets(username, smallsz, stdin);
	// Now establish the actual length of the input
	//size_t szn = (size_t)strlen(username);
    size_t szn = (size_t)strlen(username);
	// we expect something between 1 and 32
    assert(szn > 0);
    assert(szn <= smallsz);
	printf("size is %zu\n", szn);
	if(username[szn-1] == '\n') {		
    	username[szn-1] = '\0';
    	szn = (size_t)strlen(username);
    }
    
    // Now we prompt them for their real name:
    realname = malloc(smallsz);
    // realname
	printf("Enter your real name: \n");
	// Get and save the text
	fgets(realname, smallsz, stdin);
	//size_t szp = (size_t)strlen(realname);
    size_t szp = (size_t)strlen(realname);
	// we expect something between 1 and 32
    assert(szp > 0);
    assert(szp <= smallsz);
	printf("size is %zu\n", szp);
	if(realname[szp-1] == '\n') {
    	realname[szp-1] = '\0';
    	szp = (size_t)strlen(realname);
    }
	
	// Now we prompt them for their private review:
    privatereview = malloc(biggersz);
    // private review
	printf("Enter your private remarks: \n");
	// Get and save the text
	fgets(privatereview, biggersz, stdin);
	//size_t szprr = (size_t)strlen(privatereview);
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
	
    // now allocate whatever size we got from the input
    // so we can then show the size and width
    /*username = malloc(szn);
    assert(username != NULL);
    memcpy(username, newusername, szn);
    free(newusername);
    
    realname = malloc(szp);
    assert(realname != NULL);
    memcpy(realname, newrealname, szp);
    free(newrealname);
    
    privatereview = malloc(szj);
    assert(privatereview != NULL);    
    memcpy(privatereview, privatereview, szj);*/
    
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

	/*
     * Check whether these addresses are close to each other.
     */
    assert((size_t)(username + smallsz) <= (size_t)realname); 
    //assert((size_t)(realname + smallsz) <= (size_t)privatereview); 
    assert((size_t)(privatereview + biggersz) <= (size_t)publicreview);
    //assert((size_t)(username + smallsz + smallsz) > (size_t)realname); 
    //assert((ptraddr_t)(realname + szp + szp/2) > (ptraddr_t)privatereview); 
    /*assert((ptraddr_t)(privatereview + szprr + szprr/2) > (ptraddr_t)publicreview);*/
    printf("Username: %s\n", username);
    printf("Real name: %s\n", realname);
    printf("Private review: %s\n", privatereview);
    printf("Public review: %s\n", publicreview);
    
    reviewer.username = username;
    reviewer.realname = realname;
    reviewer.privatereview = privatereview;
    reviewer.publicreview = publicreview;
    reviewer.privileges = privileges;
    
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
    
    printf("Overflowing reviewer username by 1\n");
    //username[smallsz+1] = 'A';
    memset(reviewer.username + smallsz, 'A', 2);
    
    printf("username: %s\n", reviewer.username);
    printf("realname: %s\n", reviewer.realname);
    
    const size_t oversz = reviewer.realname - reviewer.username + 2 - smallsz;
    printf("Overflowing reviewer username by %zx\n", oversz);
    memset(reviewer.username + smallsz, 'A', oversz);
    
    printf("username: %s\n", reviewer.username);
    printf("realname: %s\n", reviewer.realname);
    
    printf("Overflowing private remarks by 1\n");
    memset(reviewer.privatereview + biggersz, 'A', 2);
    
    printf("privatereview: %s\n", reviewer.privatereview);
    printf("publicreview: %s\n", reviewer.publicreview);
    
    const size_t overbigger = reviewer.publicreview - reviewer.privatereview + 2 - biggersz;
    printf("Overflowing private review by %zx\n", overbigger);
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
    
    printf("privatereview: %s\n", reviewer.privatereview);
    printf("publicreview: %s\n", reviewer.publicreview);
		
	print_details(&reviewer);
	
	return 0;
}




