import { Component } from "solid-js";
import { createMockTextList } from "~/utils/text-list";
import TextListItem from "../text-list-item";
import styles from "./text-list-modal.module.css";

const TextListModal: Component = () => {
  const mockDataSize = 20;
  const mockData = createMockTextList(mockDataSize)

  return (
    <div class={styles.text_list_modal}>
      {mockData.map((textListItem, i) => (
        <TextListItem>
          {textListItem.title}
        </TextListItem>
      ))}    
    </div>
  );
};

export default TextListModal;