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
    $options['category_table'] = 'category';
    $options['category_base_path'] = '/category'; 


