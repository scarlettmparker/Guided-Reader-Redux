import React from "react";
import { LoadingState } from "../state";
import { Text as TextType } from "~/types";
import { renderAnnotatedText } from "~/utils/annotation";
import styles from "./text-modal.module.css";

interface TextModalProps {
  selectedTextId: number | null;
  text?: TextType;
}

const TextModal: React.FC<TextModalProps> = ({ selectedTextId, text }) => {
  return (
    <div className={styles.text_modal}>
      {selectedTextId !== null ? (
        text ? (
          <div
            dangerouslySetInnerHTML={{ __html: renderAnnotatedText(text) }}
          />
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
