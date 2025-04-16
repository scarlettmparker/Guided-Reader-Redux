import { Component, createEffect } from "solid-js";
import styles from './text-modal.module.css';
import { LoadingState } from "../state";
import { TextType } from "~/types";

interface TextModalProps {
  selectedTextId: number | null;
  text?: TextType;
}

const TextModal: Component<TextModalProps> = (props) => {
  createEffect(() => {
    console.log(props.text);
  })
  return (
    <div class={styles.text_modal}>
      {props.selectedTextId !== null ? (
        props.text ? (
          <div innerHTML={props.text.text} />
        ) : (
          <LoadingState>Loading text...</LoadingState>
        )
      ) : (
        <span>Select a text to begin</span>
      )}
    </div>
  )
};

export default TextModal;