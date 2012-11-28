<?php

if (!session::checkAccessControl('category_allow')){
    return;
}

include_module('category');
include_once "conf.php";
$c = new categoryControl($options);
$c->deleteCat();