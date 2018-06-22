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
      $loc_url = urlencode(htmlspecialchars('Louisiana & Lower Mississippi Valley Collections'));
      $fifth = '&Location=' . $loc_url;
      $sixth = '&CallNumber=';
      $seventh = '&Notes=';

      $obj = islandora_object_load($pid);
      $dc = $obj['DC'];
      $dc_string =$dc->content;
      $dc_xml = simplexml_load_string($dc_string);
      //$dcns = $dc_xml->getNameSpaces(TRUE);
      //$dc = $dc_xml->children($dcns['dc']);
      //dpm($dc);
      $dc_title = dc_title($dc_xml);
      dpm($dc_title);

      $mods = $obj['MODS'];
      $mods_string = $mods->content;
      // dpm($mods_string);
      $mods_xml = simplexml_load_string($mods_string);

      $call_number =
      $phys_arr = physical_arrangement($mods_xml);
      dpm('call number: ' . $call_number);
      dpm('physical arrangement: ' . $phys_arr);
      $vol_issue = vol_issue($mods_xml);
      dpm($vol_issue);
      $file_name = file_name($mods_xml);
      dpm($file_name);

      //$vol_box_reel = '&ItemVolume=';
      $call_number = '&CallNumber' . call_number($mods_xml);
      $folder = '&ItemInfo2=' . $phys_arr;

      $url = $_SERVER['SERVER_NAME'] . ':8000' . '/fedora/repository/' . $pid;
      $aeon_link =  $first . $second . $third . $fourth . $obj->label . $fifth . $folder . $call_number .  $sixth . $pid  . $seventh . $url;
      return $aeon_link;
    }
  }
}

function dc_title($dc) {
  $title = $dc->xpath('//dc:title');
  $dc_title = '';
  if(count($title) > 1){
    foreach($title as $t){
        $dc_title = (string)$t;
    }
  }
  else {
    $dc_title = (string)$title[0];
  }
  return $dc_title;
}

function file_name($mods) {
  $id = $mods->identifier;
  $file_name = '';
  foreach($id as $i) {
    $dl = (string)$i['displayLabel'];
     if($dl == 'Object File Name' || $dl == 'Item Number' )
     $file_name = $dl . ': ' . (string)$i;
  }
  return $file_name;
}

function vol_issue($mods) {
  $part = $mods->part;  //this matches all > xpath('//mods:part');
  $vol_issue = '';
  if(isset($p->detail)) {
    foreach($part as $p) {
      $label = (string)$p->detail['type'];
      $number = (string)$p->detail->number;
      $vol_issue = ucfirst($label) . ': ' . $number;
    }
  }
  else {
    $vol_issue = FALSE;
  }
  return $vol_issue;
}

function call_number($mods) {
  $loc = $mods->location;
  foreach($loc as $l) {
    if(isset($l->shelfLocator)) {
      $call_number = (string)$l->shelfLocator;
    }
    elseif(isset($l->holdingSimple)) {
      $call_number = (string)$l->holdingSimple->copyInformation->shelfLocator;
    }
    else {
      $call_number = FALSE;
    }
  }
  return $call_number;
}

function physical_arrangement($mods) {
  $related = $mods->xpath('//mods:relatedItem');
    //foreach($mods->relatedItem as $related){}
    //dpm((string)$mods->relatedItem->titleInfo['displayLabel']);
  //dpm($related);
  $ti = $related[0]->titleInfo;
  //dpm($ti);
  $phys_1 = (string)$ti->title; //(string)$ti->attributes()->displayLabel
  $phys_2 = (string)$ti->partName;
  $phys_3 = (string)$ti->partNumber;
  if(!empty($phys_2)) {
    $phys_arr = $phys_1 . ' ' . $phys_2;
  }
  elseif(!empty($phys_3)) {
    $phys_arr = $phys_1 . ' ' . $phys_3;
  }
  else {
    $phys_arr = FALSE;
  }
   return $phys_arr;
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
