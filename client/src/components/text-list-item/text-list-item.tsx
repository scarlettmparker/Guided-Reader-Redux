import { Component, JSX } from "solid-js";
import styles from './text-list-item.module.css';

interface TextListItemProps {
  children: JSX.Element | string;
}

const TextListItem: Component<TextListItemProps> = (props) => {
  const { children } = props;

  return (
    <div class={styles.text_list_item}>
      {children}
    </div>
  );
};

export default TextListItem;