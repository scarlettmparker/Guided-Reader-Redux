import React from "react";
import Header from "~/components/header";
import styles from "./reader-modal.module.css";

interface ReaderModalProps {
  children: React.ReactNode;
}

const ReaderModal: React.FC<ReaderModalProps> = ({ children }) => {
  return (
    <div className={styles.reader_modal}>
      <Header />
      {children}
    </div>
  );
};

export default ReaderModal;
