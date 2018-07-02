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
  * construct giant cronenburg string.
 */

function check_path() {
  $list = variable_get('islandora_aeon_namespace_list','test');
  $path_test = [];
  if (isset($list)) {
    $path = str_replace('/', '', request_uri());
    $not_displayed = ['collection', 'manage', 'datastream', 'regenerate', 'root', 'destination'];
    foreach($not_displayed as $check) {
      if(preg_match('/'.$check.'/', $path) !== FALSE){
        $path_test[$check] = 'true';
      }
    }
  }
  $path_check = in_array('true',$path_test);
  //check that the current item is amoung our namespace list members
  $list = explode(' ', $list);
  foreach($list as $namespace) {
    if (strpos($path, $namespace) !== FALSE && $path_check){
    return TRUE;
    }
  }
}

function aeon_link($link_type){
  $aeon_url = variable_get('islandora_aeon_url');
  $test = explode('-', $link_type);
  $auth = in_array('lsu', $test) ? 'Remote' : 'Aeon';
  $form = in_array('req', $test) ? '20' : '23';
  $formpart = '/aeon.dll?Action=10&Form=' . $form .'&Value=GenericRequestAll';
  $authpart = '/' . $auth .'Auth';
  $pid = str_replace('/islandora/object/','', request_uri());
  $link_meta = link_builder($pid);
  $aeon_link = $aeon_url . $authpart . $formpart . $link_meta;
  return $aeon_link;
}

function link_builder($pid) {
  $string ='';
  $string .= '&DocumentType=Unknown';
  $string .= '&Location=' . 'unknown';
  $obj = islandora_object_load($pid);
  $dc_xml = simplexml_load_string($obj['DC']->content);
  $mods_xml = simplexml_load_string($obj['MODS']->content);
  $title = '&ItemTitle='; //aka Title/Archival Collection Name
  $arrangement = physical_arrangement($mods_xml);
  $url = $_SERVER['SERVER_NAME'] . ':8000' . '/fedora/repository/' . $pid;
  $string .= '&ItemInfo2=' . $arrangement['Folder'];
  $string .= '&Notes=' . urlencode($arrangement['notes'] . ' ' . $url);
  $string  .= '&ItemVolume=' . $arrangement['Box'];
  //dpm($mods_xml->children());
  // dpm('physical arrangement: ' . $arrangement['Folder']);
  // dpm('notes: ' . $arrangement['notes']);
  // dpm('box nuber: ' .$arrangement['Box']);
  // dpm('call number: ' . call_number($mods_xml));
  // dpm(file_name($mods_xml));
  // dpm(vol_issue($mods_xml));
  $string .= $title . dc_title($dc_xml) . '/' . parent_coll($obj);
  $string .= vol_issue($mods_xml);
  $string .= '&SpecialRequest='. file_name($mods_xml);
  $string .= '&CallNumber=' . urlencode(call_number($mods_xml));
  return $string;
}

//ask for parent object and its label

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

//get the dc.title

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

//get the filename and its display label

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

//get vol_issue from $mods->part

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

//get call number from one of two locations ->holdingSimple or ->holdingSimple->copyInformation->shelfLocator;
// or return unkown

function call_number($mods) {
  $loc = $mods->location;
  $call_number = 'unknown';
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

//get all related item information, and place into an array that can be used by the string_building

function physical_arrangement($mods) {
  //returning array
  $info = array();
  if(isset($mods->relatedItem)){
    $related_info = $mods->relatedItem;
    foreach($related_info as $related) {
      if(isset($related->titleInfo)) {
        $phys = $related->titleInfo;
        foreach($phys as $key => $value) {
          if(isset($value['displayLabel'])) {
            $info[((string)$value['displayLabel'])][]['title'] = (string)$value->title;
            $info[((string)$value['displayLabel'])][]['partname'] = count($value->partName) < 1 ? (string)$value->partName : consolidate_multiple($value->partName);
            $info[((string)$value['displayLabel'])][]['partnum'] = count($value->partNumber) < 1 ? (string)$value->partNumber : consolidate_multiple($value->partNumber);
          }
        }
      }
    }
  }
  //find the parts of the 'Folder' array or 'Box' array are set, and return them at a flatter level of $info
  $info['Folder'] = isset($info['Folder']) ? detangle_array($info['Folder']) : '';
  $info['Box'] = isset($info['Box']) ? detangle_array($info['Box']) : '';
  //construct a string from of info's arrays (if set.)
  $phys_str = '';
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
  //chop off last ': '
  //$phys_arr = substr($phys_str, 0, -2);
  //if the box and info didn't already get put in their repsective $info['Folder'] and $info['Box']
  //put them there with this function via preg_match_all
  $info = check_notes_for_box_folder($phys_str, $info);
  return $info;
}

// extract box and folder, and replace them in the long 'notes' string

function check_notes_for_box_folder($string, $info) {
  $found = array();
  preg_match_all("/(\bFolder.\d+\b|\bBox.\d+\b)/i", $string, $found);
  $new_notes = preg_replace("/(\bFolder.\d+\b|\bBox.\d+\b)/i", "", $string);
  //take of last ':  '
  //$new_notes = substr($new_notes, 0, -3);
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

//when there are multiple partName or partNumber, put their strings together as one.
function consolidate_multiple($array) {
  $ret = array();
  $ret['all'] = '';
  foreach($array as $key => $value) {
    $ret[] = (string)$value;
    $ret['all'] .= (string)$value . ' ';
  }
  return $ret['all'];//substr($ret['all'], 0, -1);
}

// grab out the set parts of an gnarly array with empty values
function detangle_array($array) {
  $part = '';
  foreach($array as $key => $value) {
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
    $links = array(
      l('Schedule Visit (through myLSU)', aeon_link('req-lsu'), $attributes),
      l('Schedule Visit (for non LSU patrons)', aeon_link('req-non'), $attributes),
      l('Order Duplicate (through myLSU)', aeon_link('dup-lsu'), $attributes),
      l('Order Duplicate (for non LSU patrons)', aeon_link('dup-non'), $attributes),
    );
    $form['link'] = array(
      '#markup' => "<div class='aeon-desciption'><p>" . $admin_text . ": " . '<br>' . $links[0] . '<br>' . $links[1] . '<br>' . $links[2] . '<br>' . $links[3] . "</p></div>",
    );
    return $form;
}
