import React from "react";
import styles from "./text-list-modal.module.css";

interface TextListModalProps {
  children: React.ReactNode;
}

const TextListModal: React.FC<TextListModalProps> = ({ children }) => {
  return <div className={styles.text_list_modal}>{children}</div>;
};

export default TextListModal;
