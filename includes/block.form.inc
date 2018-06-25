<?php

/**
 * @file
 * This block displays a link to the aeon url
 * Block can be enabled on islandora_aeon.module
 * Change 'status' => 0, to 'status' => 1,
 */

 /**
  * Check if path matches an aeon enabled namespace
  * execute many functions to parse xml to form fields for aeon_link
  * construct giant ugly string.
 */

function check_path() {
  $list = variable_get('islandora_aeon_namespace_list','test');
  if (isset($list)) {
    $path = str_replace('/', '', request_uri());
    if(strpos($path, 'collection') === FALSE && strpos($path, 'manage') === FALSE) {
      $list = explode(' ', $list);
      foreach($list as $namespace) {
        $link = (strpos($path, $namespace) !== FALSE) ? $link = 'test' : $link = '';
      }
      $pid = str_replace('/islandora/object/','', request_uri());
      $aeon_url = variable_get('islandora_aeon_url'); //change to add namespace to end of str associate each url with series of namespaces
      $form_id = '/RemoteAuth/aeon.dll?Action=10&Form=20&Value=GenericRequestAll' . '&DocumentType=Unknown'; //aka Material Type
      //$loc_url = urlencode('Louisiana & Lower Mississippi Valley Collections');
      $loc_url = 'unknown'; //is it worth trying to add this?
      $title = '&ItemTitle='; //aka Title/Archival Collection Name
      $location = '&Location=' . $loc_url;
      $notes = '&Notes=';

      $obj = islandora_object_load($pid);
      $dc_xml = simplexml_load_string($obj['DC']->content);

      $mods_xml = simplexml_load_string($obj['MODS']->content);
      //dpm($mods_xml->children());
      $arrangement = physical_arrangement($mods_xml);
      // dpm('physical arrangement: ' . $arrangement['Folder']);
      // dpm('notes: ' . $arrangement['notes']);
      // dpm('box nuber: ' .$arrangement['Box']);
      // dpm('call number: ' . call_number($mods_xml));
      // dpm(file_name($mods_xml));
      // dpm(vol_issue($mods_xml));

      $url = $_SERVER['SERVER_NAME'] . ':8000' . '/fedora/repository/' . $pid;
      $title_and_collection = $title . dc_title($dc_xml) . '/' . parent_coll($obj);
      $vol_issue = vol_issue($mods_xml);
      $phys_arr = '&ItemInfo2=' . $arrangement['Folder'];
      $notes .= urlencode($arrangement['notes'] . ' ' . $url);
      $file_name = '&SpecialRequest='. file_name($mods_xml);
      $vol_box_reel = '&ItemVolume=' . $arrangement['Box'];
      $call_number = '&CallNumber=' . urlencode(call_number($mods_xml));

      $aeon_link =  $aeon_url . $form_id . $title_and_collection . $location . $phys_arr . $vol_box_reel . $call_number . $file_name . $notes;
      return $aeon_link;
    }
  }
}

function parent_coll($obj) {
  $parents = '';
  if(count($obj->getParents() > 1 )){
    foreach($obj->getParents() as $parent){
      $p_obj = islandora_object_load($parent);
      $parents .= $p_obj->label . ' ';
    }
  }
  else {
    $p_obj = islandora_object_load($parent);
    $parents = $p_obj->label;
  }
  return $parents;
}

function dc_title($dc) {
  //had bad luck looping through dc. this should work fine
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
  $part = $mods->part;
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
    elseif(isset($l->holdingSimple->copyInformation->shelfLocator)) {
      $call_number = (string)$l->holdingSimple->copyInformation->shelfLocator;
    }
    else {
      $call_number = 'unknown';
    }
  }
  return $call_number;
}

function physical_arrangement($mods) {
  $info = array();
  foreach($mods->relatedItem as $related) {
    if(isset($related->titleInfo)) {
      $phys = $related->titleInfo;
      foreach($phys as $key => $value) {
        if(isset($value['displayLabel'])) {
          if(count($value->partName | $value->partNumber))
          $info[((string)$value['displayLabel'])][]['title'] = (string)$value->title;
          $info[((string)$value['displayLabel'])][]['partname'] = count($value->partName) < 1 ? (string)$value->partName : return_set_array($value->partName);
          $info[((string)$value['displayLabel'])][]['partnum'] = count($value->partNumber) < 1 ? (string)$value->partNumber : return_set_array($value->partNumber);
        }
      }
    }
  }
  $info['Folder'] = isset($info['Folder']) ? return_set_part($info['Folder']) : '';
  $info['Box'] = isset($info['Box']) ? return_set_part($info['Box']) : '';
  $phys_str = '';
  $flat_array = array();
  foreach($info as $part) {
    if(is_array($part)) {
    foreach ($part as $value) {
      foreach($value as $index => $val) {
        if(!empty($val)) {
          $phys_str .= $val . ': ';
          }
        }
      }
    }
  }
  $phys_arr = substr($phys_str, 0, -2);
  $info = check_notes_for_box_folder($phys_arr, $info);
  return $info;
}

function check_notes_for_box_folder($string, $info) {
  $found = array();
  preg_match_all("/(\bFolder.\d+\b|\bBox.\d+\b)/i", $string, $found);
  $new_notes = preg_replace("/(\bFolder.\d+\b|\bBox.\d+\b)/i", "", $string);
  foreach($found as $array) {
    foreach($array as $key => $value) {
      if(strpos($value, 'Box') !== FALSE ) {
        $info['Box'] = $value;
      }
      if(strpos($value, 'Folder') !== FALSE) {
        $info['Folder'] = $value;
      }
    }
  }
  $info['notes'] = $new_notes;
  return $info;
}

function return_set_array($array) {
  $ret = array();
  $ret['all'] = '';
  foreach($array as $key => $value) {
    $ret[] = (string)$value;
    $ret['all'] .= (string)$value . ' ';
  }
  // dpm($ret);
  return substr($ret['all'], 0, -1);
}

function return_set_part($array) {
  $part = '';
  foreach ($array as $key => $value) {
    if(is_array($value)){
      foreach ($value as $k => $v) {
        if(isset($v)){
          $part = $v;
        }
      }
    }
  }
  return $part;
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
