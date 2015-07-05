<?php
/** Drupal 7 hooks. */


/**
 * Implements hook_permission().
 */
function opengraph_meta_permission() {
    return array(
        OPENGRAPH_META_PERM_ADMIN => array(
            'title' => t('Administer Open Graph meta tags'),
            'description' => t('Administer default Open Graph meta tags options'),
        ),
        OPENGRAPH_META_PERM_EDIT => array(
            'title' => t('Edit Open Graph meta tags'),
            'description' => t('Edit Open Graph meta tags on a node'),
        ),
    );
}

/**
 * Implements hook_metatag_info_alter().
 */
function opengraph_meta_metatag_info_alter(&$info) {
  $info['tags']['og:image']['class'] = 'OpengraphImageMetaTag';
}
