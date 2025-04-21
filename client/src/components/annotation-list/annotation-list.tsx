import React from "react";
import { Card, CardBody, CardHeader } from "~/components/card";
import HideIcon from "~/components/hide-icon";
import styles from "./annotation-list.module.css";

interface AnnotationListProps {
  className?: string;
  children: React.ReactNode;
}

const AnnotationList: React.FC<AnnotationListProps> = ({ className = "", children }) => {
  return (
    <Card className={`${styles.annotation_list} ${className}`}>
      <CardHeader className={styles.annotation_list_header}>
        <HideIcon reverse={true} className={styles.hide_icon}>
          X
        </HideIcon>
        <span>Annotations</span>
      </CardHeader>
      <CardBody className={styles.annotation_list_body}>{children}</CardBody>
    </Card>
  );
};

export default AnnotationList;
