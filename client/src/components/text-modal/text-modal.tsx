import { Component } from "solid-js";
import { LoadingState } from "../state";
import { Text as TextType } from "~/types";
import { renderAnnotatedText } from "~/utils/annotation";
import styles from "./text-modal.module.css";

interface TextModalProps {
  selectedTextId: number | null;
  text?: TextType;
}

const TextModal: Component<TextModalProps> = (props) => {
  return (
    <div class={styles.text_modal}>
      {props.selectedTextId !== null ? (
        props.text ? (
          <div innerHTML={renderAnnotatedText(props.text)} />
        ) : (
          <LoadingState>Loading text...</LoadingState>
        )
      ) : (
        <span>Select a text to begin</span>
      )}
    </div>
  );
};

export default TextModal;
