<?php

/**
 * @file
 * can be found at: Admin -> Islandora -> Islandora Utility Modules -> Aeon
 */

/**
 * Admin form and submission handler.
 */

function islandora_aeon_admin_form($form, &$form_state) {
  $ns_list = variable_get('islandora_aeon_namespace_list');
  $ns_list = (isset($ns_list) ? $ns_list : '');
  $form['list'] = array(
    '#title' => 'Namespace List',
    '#type' => 'textarea',
    '#value' => $ns_list,
    '#description' => 'Enter namespace(s) you would like directed to the aeon request service.',
    '#default_value' => 'test-namespace',
  );
  $default_text = "You may order a high-resolution, digital copy of this item. LSU Special Collections uses the Aeon request management system. If you do not yet have an account, you will be prompted to create one. You will need this account to process your order. More information about requesting information, and creating account account can be found here";
  $admin_text = variable_get('islandora_aeon_admin_text');
  $text = (isset($admin_text)) ? $admin_text : $default_text;
  $form['context'] = array(
    '#title' => 'Explain contetx of link',
    '#type' => 'textarea',
    '#default_value' => $text,
  );
  //Will need multiple aeon service links per institution in the future.
  $form['link'] = array(
    '#title' => 'Link to Aeon service',
    '#type' => 'textfield',
    '#default_value' => 'https://example.org', //example path to aeon site
  );
  $form['submit'] = array(
   '#type' => 'submit',
   '#value' => t('Submit'),
  );
  return $form;
}

/**
 * Form submission handler.
 */


function islandora_aeon_admin_form_submit($form, &$form_state) {
  variable_set('islandora_aeon_url', $form_state['input']['link']);
  variable_set('islandora_aeon_namespace_list', $form_state['input']['list']);
  variable_set('islandora_aeon_admin_text', $form_state['input']['context']);
}
