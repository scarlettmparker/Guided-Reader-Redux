import React from "react";
import TextListModal from "~/components/text-list-modal";
import Header from "~/components/header";
import HideIcon from "~/components/hide-icon";
import styles from "./text-list.module.css";

interface TextListProps {
  children: React.ReactNode;
}

const TextList: React.FC<TextListProps> = ({ children }) => {
  return (
    <div className={styles.text_list}>
      <Header>
        <HideIcon reverse={true} className={styles.hide_icon} />
        <span>Texts (κείμενα)</span>
      </Header>
      <TextListModal>{children}</TextListModal>
    </div>
  );
};

export default TextList;
