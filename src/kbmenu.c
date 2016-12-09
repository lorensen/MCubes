/* display a menu on the terminal. menu item names are limited to 40 chars */

#include <stdio.h>
#include "kbmenu.h"

struct  menu_info {                     /* menu information     */
	MENU_ITEM       *items;
	int     item_count;
	};



/************
 *          *
 *** MENU ***
 *          *
 ************/

menu (menu_items)

	MENU_ITEM       menu_items[];   /* array of menu items    */
{
	int     count;                  /* the number of menu items     */
	int     item;
	struct  menu_info       menu;   /* menu information             */
	int     (*fcn)();


	/*
	 * find number of menu items
	 */

	for (count = 0; strlen(menu_items[count].item_name) != 0; count++);
	menu.item_count = count ;

	/*
	 * build menu list
	 */

	menu.items = menu_items;

	for (;;) {

		/*
		 * display menu
		 */

		disp_menu (&menu) ;
		item = get_item (&menu);

		/*
		 * exit if there is no function to call
		 */

		fcn = menu_items[item].item_fcn ;
		if (fcn == 0) {
			break;
		}

		/*
		 * otherwise, we call the routine
		 *     exit if routine returns non-zero.
		 */

		if ((*fcn)() != 0) {
			break;
		}
	}

	return (item+1);

} /* menu */

/*****************
 *               *
 *** GET_ITEM  ***
 *               *
 *****************/

/* return the item selected from the menu */

get_item (menu)
	struct  menu_info       *menu;
{
	int     got;
	int     item ;
	char    line[80];

	do {
		printf ("Selection? ") ;
		if (gets (line) == NULL) exit ();
		got = sscanf (line, "%d", &item) ;
	} while (got != 1 || item < 0 || item >= menu->item_count) ;

	return (item);

} /* get_item */


/*****************
 *               *
 *** DISP_MENU ***
 *               *
 *****************/

disp_menu (menu)
	struct  menu_info       *menu;
{
	int     item ;

	printf ("\n\n") ;

	for (item = 0; item < menu->item_count; item++){
		printf ("%d\t%s\n",item, (menu->items + item)->item_name) ;
	}
	printf ("\n") ;

} /* disp_menu */


