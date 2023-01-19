#ifdef __CHERI_PURE_CAPABILITY__
#include "../../include/common.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct reviewer
{
	char *username;
	char *realname;
	char *privatereview;
	char *publicreview;
	char *privileges;
};

bool can_write(struct reviewer *rv) {
	bool canwrite = true;
	
	char *pt = strchr(rv->privileges, 'w');
	if(pt == NULL)
		return false;
		
	return canwrite;
}

void change_privatereview(struct reviewer *rv, char* privatereview)
{
	if(can_write(rv))
		rv->privatereview = privatereview;
	else
		printf("You don't have permission to change this private review.\n");
}

void change_publicreview(struct reviewer *rv, char* publicreview)
{
	rv->publicreview = publicreview;
}

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
/*
 * Traverse the struct to set the permissions.
 */
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