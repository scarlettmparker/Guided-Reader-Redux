import { Component, JSX } from 'solid-js';
import { createMockTextList } from "~/utils/text-list";
import Header from '~/components/header';
import HideIcon from '~/components/hide-icon';
import TextListModal from '~/components/text-list-modal';

import styles from './text-list.module.css';

interface TextListProps {
  children: (mockData: ReturnType<typeof createMockTextList>) => JSX.Element;
}

const TextList: Component<TextListProps> = (props) => {
  return (
    <div class={styles.text_list}>
      <Header>
        <HideIcon reverse={true} class={styles.hide_icon}/>
        <span>
          Texts (κείμενα)
        </span>
      </Header>
      <TextListModal>{props.children}</TextListModal>
    </div>
  );
};

export default TextList;