<?php

include_once "coslib/rb_helpers.php";
include_once "coslib/form_helpers.php";
RB::connect();


class category {
    
    public $table = null;
    public function __construct($options) {
        $this->table = $options['table'];
        $this->options = $options;
    }
    
    public function setTable ($table) {
        $this->table = $table;
    }
    
    public function add ($values) {    
        $db = new db();
        return $db->insert($this->table, $values);

    }
    
    public function delete ($id) {
        db::begin();
        event::getTriggerEvent(
                config::getModuleIni('category_events'), array (
                    'action' => 'category_delete',
                    'id' => $id)
                );
        dbQ::setDelete($this->table)->filter('id =', $id)->exec();
        db::commit();
      
    }
    
    public function update ($id, $values) {
        return dbQ::setUpdate($this->table)->setUpdateValues($values)->filter('id=', $id)->exec();
    }
    
    public function get($id) {
        return dbQ::setSelect($this->table)->filter('id = ', $id)->fetchSingle();
    }
    
    public function getLevel($id) {
        return dbQ::setSelect($this->table)->filter('parent =', $id)->order('sort')->fetch();
    }
    
    public function getLevelAsIds($id) {
        $level = self::getLevel($id);
        $ary = array ();
        foreach ($level as $l) {
            $ary[] = $l['id'];
        }
        return $ary;
    }
    
    public function getChildren ($parent) {
        return dbQ::setSelect($this->table)->filter('parent =', $parent)->order('sort')->fetch();
    }
    
    public function getParent($id) {
        $row = dbQ::setSelect($this->table)->filter('id =', $id)->fetchSingle();
        if (empty($row)) {
            return 0;
        }
        return $row['parent'];
    }
    
    public function getTrail ($id) {
        static $trail = array ();
        $parent = $this->getParent($id);
        if ($parent) { 
            $trail[] = $parent;
            $this->getTrail($parent);
        } 
        return $trail;
    }
    
    public function getTrailHuman ($id) {
        $trail = array_reverse($this->getTrail($id));

        $ary = array ();
        foreach ($trail as $key => $val) {
            $row = $this->get($val);
            $ary[] = $row;
        }
        if ($id != 0) {
            $ary[] = $this->get($id);
        }
        return $ary;
    }
    
    public function getTrailHTML ($id) {
        $trail = $this->getTrailHuman($id);      
        $str = '';    
        if (count($trail) == 0) {
            return "Top";
        }
        
        $str.=  html::createLink($this->options['path'] .  "/admin/add_cat", 'Top');       
        $num = count($trail);

        
        foreach ($trail as $row) {            
            $num--;
            if (!$num) {
                $str.=  " &lt;&lt; " . $row['title'];
            } else {
                $str.=  " &lt;&lt; " . html::createLink($this->options['path'] . "/admin/add_sub/$row[id]", html::specialEncode($row['title']));
            }
        }   
        return $str;
    }
    
    public function getOptgroupArray () { 
        
        $top = $this->getLevel(0);

        $ary = array ();
        foreach ($top as $parent) {
            $set = array ();
            $set['optgroup'] = $parent['title'];
            $set['id'] = $parent['id'];
            $set['options'] = $this->getChildren($parent['id']);
            $ary[] = $set;

        }
        return $ary;
    }
    
    public function exists ($id) {
        $ary = $this->getOptgroupArray();
        foreach ($ary as  $group) {
            foreach ($group['options'] as $item) {
                if (isset($item['id'])) {
                    if ($item['id'] == $id) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}

class categoryForm {
    
    public function formAdd () {
        $_POST = html::specialEncode($_POST);
        $f = new html();
        $f->init(null, 'category_add');
        $f->formStart();
        $f->legend(lang::translate('category_form_add_legend'));
        $f->label('title', lang::translate('category_form_add_title'));
        $f->text('title');
        $f->submit('category_add', lang::translate('category_form_add_submit'));
        $f->formEnd();
        echo $f->getStr();
    }
    
    public function formDelete () {
        echo formHelpers::confirmDeleteForm('category_delete', lang::translate('category_form_delete_legend'));
    }
    
    public function formUpdate ($values) {
        $_POST = html::specialEncode($_POST);
        $f = new html();
        $f->init($values, 'category_update');
        $f->formStart();
        $f->legend(lang::translate('category_form_update_legend'));
        $f->label('title', lang::translate('category_form_title'));
        $f->text('title', null);
        $f->submit('category_update', lang::translate('category_form_update_submit'));
        $f->formEnd();
        echo $f->getStr();
    }    
}

class categoryControl {
    
    public static $options = array ();
    function __construct($options = null) {
        self::$options = $options;

    }
    
    public function addParent () {

        headline_message(lang::translate('category_add_title'));       
        
        $f = new categoryForm();
        $c = new category(self::$options);
        
        $trail = $c->getTrailHTML(0);     
        echo $trail;
        
        if (isset($_POST['category_add'])) {
            $_POST = html::specialDecode($_POST);
            
            $values = db::prepareToPostArray(array ('title'));
            $values['sort'] = 255;
            $values['parent'] = 0;
            $res = $c->add($values);
            if ($res) {
                session::setActionMessage(lang::translate('category_add_action'));
                http::locationHeader(self::$options['path'] . '/admin/add_cat');
            } else {

            }
        }

        echo $f->formAdd();

        $func = function  ($id, $text) {
            $edit = html::createLink(categoryControl::$options['path'] . "/admin/edit_cat/$id", lang::system('edit'));
            $del = html::createLink(categoryControl::$options['path']  . "/admin/delete_cat/$id", lang::system('delete'));
            $sub = html::createLink(categoryControl::$options['path']  ."/admin/add_sub/$id", lang::translate('category_add_sub'));
            return $text  . " " . $edit . MENU_SUB_SEPARATOR . $del . MENU_SUB_SEPARATOR . $sub;
        };
        
        $options =
            array (
                'table' => self::$options['table'], 
                'title' => 'title',
                'field' => 'sort',
                'where_sql' => "`parent` = 0 ",
                'function' => $func
             );

        include_module('jquerysort');
        $j = new jquerysort($options);
        $j->setJs();
        echo $j->getHTML();
    }
    
    public function addSub () {
        headline_message(lang::translate('category_add_sub_title'));

        $parent = URI::getInstance()->fragment(3);
        $f = new categoryForm();
        
        $c = new category(self::$options);
        $trail = $c->getTrailHTML($parent);       
        echo $trail;
        
        if (isset($_POST['category_add'])) {
            $_POST = html::specialDecode($_POST);
            $values = db::prepareToPostArray(array ('title'));
            $values['sort'] = 255;
            $values['parent'] = $parent;
            $res = $c->add($values);
            if ($res) {
                session::setActionMessage(lang::translate('category_add_sub_action'));
                http::locationHeader($_SERVER['REQUEST_URI']);
            } else {

            }
        }

        echo $f->formAdd();
        
        $func = function ($id, $text) {
            $edit = html::createLink(categoryControl::$options['path']  . "/admin/edit_cat/$id", lang::system('edit'));
            $del = html::createLink(categoryControl::$options['path']  . "/admin/delete_cat/$id", lang::system('delete'));
            if (config::getModuleIni('category_allow_sub')) {
                $sub = MENU_SUB_SEPARATOR . html::createLink(categoryControl::$options['path']  . "/admin/add_sub/$id", lang::translate('category_add_sub'));
            } else {
                $sub = '';
            }
            return html::specialEncode($text) . " " . $edit . MENU_SUB_SEPARATOR . $del . $sub;
        } ;
        
        
        $options =
            array (
                'table' => self::$options['table'], 
                'title' => 'title',
                'field' => 'sort',
                'where_sql' => "`parent` = $parent ",
                'function' => $func
             );
        
        include_module('jquerysort');
        $j = new jquerysort($options);
        $j->setJs();
        echo $j->getHTML();
    }
    
    public function deleteCat () {
        headline_message(lang::translate('category_delete_title'));
        $id = URI::getInstance()->fragment(3);

        $f = new categoryForm();
        $c = new category(self::$options);
        
        $trail = $c->getTrailHTML($id);       
        echo $trail;
        
        if (isset($_POST['category_delete'])) {

            $children = $c->getChildren($id);
            if (!empty($children)) {
                view_form_errors(lang::translate('category_error_has_children'));
            } else {
                $values = db::prepareToPostArray(array ('title'));
                $res = $c->delete($id);
                session::setActionMessage(lang::translate('category_deleted'));
                http::locationHeader(self::$options['path'] . '/admin/add_cat');
            }
        }
        echo $f->formDelete(); 
    }
    
    public function editCat () {
        headline_message(lang::translate('category_edit_title'));
        $id = URI::getInstance()->fragment(3);

        $f = new categoryForm();
        $c = new category(self::$options);
        
        $trail = $c->getTrailHTML($id);       
        echo $trail;

        if (isset($_POST['category_update'])) {
            $_POST = html::specialDecode($_POST);
            $values = db::prepareToPostArray(array ('title'));
            $res = $c->update($id, $values);
            if ($res) {
                session::setActionMessage(lang::translate('category_edit_action_message'));
                http::locationHeader($_SERVER['REQUEST_URI']);
            } else {
                log::debug('Should not happen');
            }
        }

        $values = $c->get($id);
        $values = html::specialEncode($values);
        echo $f->formUpdate($values);
    }
}
