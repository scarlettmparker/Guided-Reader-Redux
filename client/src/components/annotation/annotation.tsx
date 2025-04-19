import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import LikeButton from "~/components/like-button";
import { Annotation as AnnotationType } from "~/types";
import styles from './annotation.module.css';

interface AnnotationProps {
  annotation: AnnotationType;
}

const Annotation = () => {
  return (
    <Card>
      <CardHeader
        class={styles.annotation_header}
      >
        <span class={styles.username}>username</span>
        <span class={styles.annotation_date}>
          1 month ago
        </span>
      </CardHeader>
      <CardBody>
        It's common in Greek for names to be preceded by a definite article
      </CardBody>
      <CardFooter class={styles.annotation_footer}>
        <LikeButton
          class={styles.like_button}
        />
        <span class={styles.likes_count}>0</span>
        <LikeButton
          reverse={true}
        />
      </CardFooter>
    </Card>
  );
};

export default Annotation;