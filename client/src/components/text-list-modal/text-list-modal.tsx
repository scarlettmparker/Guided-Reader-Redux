import { Component, JSX } from "solid-js";
import styles from "./text-list-modal.module.css";

interface TextListModalProps {
  children: JSX.Element;
}

const TextListModal: Component<TextListModalProps> = (props) => {
  return (
    <div class={styles.text_list_modal}>
      {props.children}    
    </div>
  );
};

export default TextListModal;