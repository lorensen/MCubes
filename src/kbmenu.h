/* structure for a menu item    */
typedef struct  {
	char    *item_name ;    /* string to be displayed       */
	int     (*item_fcn)() ; /* function to be invoked       */
	} MENU_ITEM ;

