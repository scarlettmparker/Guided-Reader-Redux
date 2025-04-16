import { Component, JSX } from "solid-js";
import styles from './text-list-item.module.css';

interface TextListItemProps {
  children: JSX.Element | string;
  onClick: () => void;
  onMouseOver: () => void;
}

const TextListItem: Component<TextListItemProps> = (props) => {
  const { children, onClick, onMouseOver } = props;

  return (
    <div 
      class={styles.text_list_item} 
      onClick={onClick}
      onMouseOver={onMouseOver}
    >
      {props.children}
    </div>
  );
};

export default TextListItem;