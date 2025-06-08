import React from "react";
import {useTranslations} from 'next-intl';
import TextListModal from "@/components/index/text-list-modal";
import Header from "@/components/header";
import HideIcon from "@/components/hide-icon";
import styles from "./text-list.module.css";

interface TextListProps {
  children: React.ReactNode;
}

const TextList: React.FC<TextListProps> = ({ children }) => {
  const t = useTranslations("home");

  return (
    <div className={styles.text_list}>
      <Header>
        <HideIcon reverse={true} className={styles.hide_icon} />
        <span>{t("texts")}</span>
      </Header>
      <TextListModal>{children}</TextListModal>
    </div>
  );
};

export default TextList;
