import { Component } from 'solid-js';

import Header from '~/components/header';
import HideIcon from '~/components/hide-icon';
import TextListModal from '~/components/text-list-modal';

import styles from './text-list.module.css';

const TextList: Component = () => {
  return (
    <div class={styles.text_list}>
      <Header>
        <HideIcon reverse={true} class={styles.hide_icon}/>
        <span>
          Texts (κείμενα)
        </span>
      </Header>
      <TextListModal />
    </div>
  );
};

export default TextList;