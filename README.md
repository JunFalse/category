Generic category system with limitless categories and allowing of 
sorting the categories using the jquerysort module (drag and drop). 

In order to connect this module to any other module you will need
to put the controls from category/admin dir into your own modules 
admin folder and then edit the admin/conf.php file. 

If you are using the module in e.g. a module called blog you 
will need to set the following two values to something like
the folloing in blog/admin/conf.php:

    <?php

    $options = array ();   
    $options['table'] = 'shop_category';
    $options['path'] = '/shop';

### Redbeans sets options for utf8 which can ruin you utf8

To change this:  

alter table shop_category change title title varchar(255) DEFAULT NULL;

### Create able manual

CREATE TABLE `shop_category` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `title` varchar(255) DEFAULT NULL,
  `sort` tinyint(3) unsigned DEFAULT NULL,
  `parent` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB;

