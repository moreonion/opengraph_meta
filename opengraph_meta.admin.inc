<?php

require_once('opengraph_meta.common.inc');


/**
 * Menu callback: settings form.
 */
function opengraph_meta_settings_form() {

  $types = OpenGraphMetaDrupalLayer::get_node_types();

  $type_options = array();
  foreach ($types as $id => $data) {
    $type_options[$id] = $data->name;
  }

  $form[OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED] = array(
    '#title' => t('Enable for the following content types'),
    '#type' => 'checkboxes',
    '#options' => $type_options,
    '#description' => t('The content types for which to enable Open Graph meta tags. If none are selected then tags will be enabled for ALL content types.'),
    '#default_value' => variable_get(OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED, array()),
  );

  return $form;
}


function opengraph_meta_settings_form_submit($form, &$form_state) {

  $values = $form_state['values'];

  // save content types enabled
  variable_set(OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED, $values[OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED]);

  // save og:type for each content type
  $types = OpenGraphMetaDrupalLayer::get_node_types();
  foreach ($types as $id => $data) {
    OpenGraphMetaDrupalLayer::update_default_ogtype_for_node_type($id, $values);
  }

  // success
  drupal_set_message("All Open Graph Meta Tags options have been updated");
}



