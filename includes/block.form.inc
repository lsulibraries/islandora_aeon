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
//       $second = '/Aeon.dll?Action=10&Form=20&Value
// =GenericRequestDigitalImage&genre=default&Item
// Title=Digital%20Repository%20Request&CallNumber=';
//       $third = '&Location=';
//       $fourth = 'http://' . $_SERVER['SERVER_NAME'];
//       $fifth = '/fedora/repository/';
//       $sixth = '&Notes=';



      $second = '/RemoteAuth/aeon.dll?Action=10&Form=20&Value=GenericRequestAll';
      $third = '&DocumentType=Unknown'; //aka Material Type
      $fourth = '&ItemTitle='; //aka Title/Archival Collection Name
      $loc_url = urldecode('Louisiana & Lower Mississippi Valley Collections');
      $fifth = '&Location=' . $loc_url;
      $sixth = '&CallNumber=';
      $seventh = '&Notes=';

      $obj = islandora_object_load($pid);
      $dc = $obj['DC'];
      $dc_string =$dc->content;
      $dc_xml = simplexml_load_string($dc_string);
      //this step not work...
      $dc_title = $dc_xml->{'oai_dc:dc'}->{'dc:title'};
      dpm($dc_title);
      //$dc_title = somexpath magick
      $mods = $obj['MODS'];
      $mods_string = $mods->content;
      $mods_xml = simplexml_load_string($mods_string);
      dpm($mods_xml->xpath('/mods/location/holdingSimple/copyInformation/shelfLocator'));
      //$obj->models;
      //$obj->owner;
      $url = $_SERVER['SERVER_NAME'] . ':8000' . '/fedora/repository/' . $pid;
      $aeon_link =  $first . $second . $third . $fourth . $obj->label . $fifth . $sixth . $pid  . $seventh . $url;
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
  $admin_text = variable_get('islandora_aeon_admin_text');
  $form['link'] = array(
    '#markup' => "<div class='aeon-desciption'><p>" . $admin_text . ": " . l('Place Order', check_path(), $attributes) . "</p></div>",
  );
  return $form;
}
