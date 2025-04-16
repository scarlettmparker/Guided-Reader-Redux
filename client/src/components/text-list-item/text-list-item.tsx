import { Component, JSX } from "solid-js";
import styles from "./text-list-item.module.css";

interface TextListItemProps {
  class?: () => string;
  children: JSX.Element | string;
  onClick: () => void;
  onMouseOver: () => void;
}

const TextListItem: Component<TextListItemProps> = (props) => {
  const { class: class_ = () => "", children, onClick, onMouseOver } = props;

  return (
    <div
      class={`${styles.text_list_item} ${class_()}`}
      onClick={onClick}
      onMouseOver={onMouseOver}
    >
      {props.children}
    </div>
  );
};

export default TextListItem;
