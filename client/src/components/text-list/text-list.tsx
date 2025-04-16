import { Component, JSX } from "solid-js";
import TextListModal from "~/components/text-list-modal";
import Header from "~/components/header";
import HideIcon from "~/components/hide-icon";
import styles from "./text-list.module.css";

interface TextListProps {
  children: JSX.Element;
}

const TextList: Component<TextListProps> = (props) => {
  return (
    <div class={styles.text_list}>
      <Header>
        <HideIcon reverse={true} class={styles.hide_icon} />
        <span>Texts (κείμενα)</span>
      </Header>
      <TextListModal>{props.children}</TextListModal>
    </div>
  );
};

export default TextList;
