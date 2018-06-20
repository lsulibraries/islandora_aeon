<?php

/**
 * @file
 * This block displays a link to the aeon url
 * Block can be enabled on islandora_aeon.module
 * Change 'status' => 0, to 'status' => 1,
 */


 /**
  * Check if path matches an aeon enabled namespace
 */
 function check_path() {
   $list = variable_get('islandora_aeon_namespace_list','test');
   if (isset($list)) {
     $path = str_replace('/', '', request_uri());
     if(strpos($path, 'collection') === FALSE) {
       $list = explode(' ', $list);
       foreach($list as $namespace) {
         $link = (strpos($path, $namespace) !== FALSE) ? $link = 'test' : $link = '';
       }
       $pid = str_replace('/islandora/object/','', request_uri());
       $first = variable_get('islandora_aeon_url');
       $second = '/Aeon.dll?Action=10&Form=20&Value
 =GenericRequestDigitalImage&genre=default&Item
 Title=Digital%20Repository%20Request&CallNumber=';
       $third = '&Location=';
       $fourth = 'http://' . $_SERVER['SERVER_NAME'];
       $fifth = '/fedora/repository/';
       $sixth = '&Notes=';
       $obj = islandora_object_load($pid);
       $aeon_link =  $first . $second . $third . $pid  . $fourth. $fifth . $pid . $sixth . $obj->label;
       return $aeon_link;
     }
   }
 }



function islandora_aeon_link_form($form, &$form_state) {
  $attributes = array(
    'attributes' => array(
      'class' => array('aeon-link'),
    ),
  );
  $form['link'] = array(
    '#markup' => "<div class='aeon-desciption'><p>You may order a high-resolution,
digital copy of this image. Following the \"Place Order\" link below will take
you to our Aeon request management system. If you have not used this system,
you will be prompted to create a new account. You will need this account
to process your order. " . l('Place Order', check_path(), $attributes) ."</p></div>",
  );
  return $form;
}
