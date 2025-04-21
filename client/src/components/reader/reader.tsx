import React, { useState, useEffect, useContext } from "react";

import {
  Text as TextType,
  Annotation as AnnotationType,
  AnnotationResponse,
} from "~/types";

import { TextContext } from "~/contexts/text-context";
import { AnnotationController, TitlesController } from "~/utils/api";
import { handleAnnotationClick } from "~/utils/annotation";
import { shouldFetchText, getFromCache, cacheText } from "~/utils/text";

import { LoadingState, ErrorState } from "~/components/state";
import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import TextListItem from "~/components/text-list-item";
import Annotation from "~/components/annotation";
import AnnotationList from "~/components/annotation-list";

import textListItemStyles from "~/components/text-list-item/text-list-item.module.css";
import styles from "./reader.module.css";

const Reader: React.FC = () => {
  const [selectedTextId, setSelectedTextId] = useState<number | null>(null);
  const [hoveredTextId, setHoveredTextId] = useState<number | null>(null);
  const [selectedTextData, setSelectedTextData] = useState<
    TextType | undefined
  >();

  const [annotations, setAnnotations] = useState<AnnotationResponse[]>([]);
  const [selectedAnnotation, setSelectedAnnotation] =
    useState<AnnotationType | null>(null);
  const [titles, setTitles] = useState<{
    loading: boolean;
    error?: Error;
    message?: any[];
  }>({
    loading: true,
  });

  useEffect(() => {
    // Simulating the fetch of titles
    TitlesController.getTitles()
      .then((data) => setTitles({ loading: false, message: data.message }))
      .catch((err) => setTitles({ loading: false, error: err }));
  }, []);

  // useEffect to handle fetch when hoveredTextId changes
  useEffect(() => {
    const fetchHoveredText = async (id: number | null) => {
      if (id === selectedTextId) {
        return undefined;
      }

      const cached = getFromCache(id!);
      if (cached) {
        return { message: [cached] };
      }

      if (shouldFetchText(id)) {
        const text = await cacheText(id!);
        return text ? { message: [text] } : undefined;
      }

      return undefined;
    };

    if (hoveredTextId !== null) {
      fetchHoveredText(hoveredTextId);
    }
  }, [hoveredTextId, selectedTextId]);

  // Handle selection changes and fetch text if needed
  useEffect(() => {
    const id = selectedTextId;
    setSelectedAnnotation(null);

    if (id === null) {
      return setSelectedTextData(undefined);
    }

    const cached = getFromCache(id);
    if (cached) {
      setSelectedTextData(cached);
    } else if (shouldFetchText(id)) {
      cacheText(id).then((text) => {
        if (text && id === selectedTextId) {
          setSelectedTextData(text);
        }
      });
    }
  }, [selectedTextId]);

  // Handle loading annotations upon click
  useEffect(() => {
    const annotation = selectedAnnotation;
    if (annotation) {
      AnnotationController.getAnnotations(
        annotation.text_id,
        annotation.start,
        annotation.end,
      ).then((data) => {
        setAnnotations(data?.message || []);
      });
    } else {
      setAnnotations([]);
    }
  }, [selectedAnnotation]);

  useEffect(() => {
    const handleClick = (e: MouseEvent) => {
      handleAnnotationClick(
        e,
        setSelectedAnnotation,
        selectedTextData?.annotations || [],
      );
    };

    document.addEventListener("click", handleClick);
    return () => {
      document.removeEventListener("click", handleClick);
    };
  }, [selectedTextData]);

  return (
    <TextContext.Provider value={{ setSelectedTextId }}>
      <div className={styles.reader}>
        <TextList>
          {titles.loading ? (
            <LoadingState>Loading...</LoadingState>
          ) : titles.error ? (
            <ErrorState>Error: {titles.error.message}</ErrorState>
          ) : (
            titles.message?.map((item) => (
              <TextListItem
                key={item.id}
                className={
                  item.id === selectedTextId ? textListItemStyles.selected : ""
                }
                onClick={() => setSelectedTextId(item.id)}
                onMouseOver={() => setHoveredTextId(item.id)}
              >
                {item.title}
              </TextListItem>
            ))
          )}
        </TextList>
        <ReaderModal>
          <TextModal selectedTextId={selectedTextId} text={selectedTextData} />
        </ReaderModal>
      </div>
      {selectedAnnotation !== null && annotations.length > 0 && (
        <AnnotationList className={styles.annotation_list}>
          {annotations.map((annotation, id) => (
            <Annotation key={id} annotation={annotation} />
          ))}
        </AnnotationList>
      )}
    </TextContext.Provider>
  );
};

export default Reader;
