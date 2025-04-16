import { Component, JSX } from "solid-js";
import { createMockTextList } from "~/utils/text-list";
import styles from "./text-list-modal.module.css";

interface TextListModalProps {
  children: (mockData: ReturnType<typeof createMockTextList>) => JSX.Element;
}

const TextListModal: Component<TextListModalProps> = (props) => {
  const mockData = createMockTextList(20);
  
  return (
    <div class={styles.text_list_modal}>
      {props.children(mockData)}    
    </div>
  );
};

export default TextListModal;