import { Component, JSX } from "solid-js";
import styles from './text-list-item.module.css';

interface TextListItemProps {
  children: JSX.Element | string;
  onClick: () => void;
}

const TextListItem: Component<TextListItemProps> = (props) => {
  const { children, onClick } = props;

  return (
    <div class={styles.text_list_item} onClick={onClick}>
      {children}
    </div>
  );
};

export default TextListItem;