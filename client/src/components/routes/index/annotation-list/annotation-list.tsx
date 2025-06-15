import React from "react";
import { Card, CardBody } from "~/components/card";
import styles from "./annotation-list.module.css";

interface AnnotationListProps {
  className?: string;
  children: React.ReactNode;
}

const AnnotationList: React.FC<AnnotationListProps> = ({
  className = "",
  children,
}) => {
  const [header, ...rest] = React.Children.toArray(children);

  return (
    <Card className={`${styles.annotation_list} ${className}`}>
      {header}
      <CardBody className={styles.annotation_list_body}>{rest}</CardBody>
    </Card>
  );
};

export default AnnotationList;
