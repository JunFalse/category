<?php

if (!session::checkAccessControl('category_allow')){
    moduleloader::setStatus(403);
    return;
}

include_module('category');
include_once "conf.php";
$c = new categoryControl($options);
$c->addSub();
